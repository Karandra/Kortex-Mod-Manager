#include "stdafx.h"
#include "MainFileSystem.h"
#include <Kortex/ModManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/Application.hpp>
#include "VirtualFileSystem/Mirror.h"
#include "VirtualFileSystem/MultiMirror.h"
#include "VirtualFileSystem/Convergence.h"
#include "VirtualFileSystem/VirtualFSEvent.h"
#include "DefaultModManager.h"
#include "Utility/Log.h"
#include <KxFramework/KxProcess.h>
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxProgressDialog.h>
#include <Kx/Async.hpp>

namespace
{
	enum class State
	{
		ShowDialog,
		Run,
		HideDialog
	};

	bool CheckMountPoint(const wxString& folderPath)
	{
		return KxFileFinder::IsDirectoryEmpty(folderPath);
	}
	void AddIfNotEmpty(KxStringVector& items, const wxString& path)
	{
		if (!CheckMountPoint(path))
		{
			items.push_back(path);
		}
	}
}

namespace Kortex::ModManager
{
	KxStringVector MainFileSystem::CheckMountPoints() const
	{
		// Main mods
		KxStringVector items;
		AddIfNotEmpty(items, IGameInstance::GetActive()->GetVirtualGameDir());

		// Mirrored locations
		for (const MirroredLocation& locaton: m_Manager.GetOptions().GetMirroredLocations())
		{
			AddIfNotEmpty(items, locaton.GetTarget());
		}

		return items;
	}
	void MainFileSystem::InitMainVirtualFolder()
	{
		m_Convergence.reset();

		KxStringVector folders;
		for (const IGameMod* mod: m_Manager.GetMods(GetModsFlags::Everything|GetModsFlags::ActiveOnly))
		{
			folders.push_back(mod->GetModFilesDir());
		}

		const IGameInstance* instance = IGameInstance::GetActive();
		auto fileSystem = std::make_unique<VirtualFileSystem::Convergence>(instance->GetVirtualGameDir(), m_Manager.GetWriteTarget().GetModFilesDir());
		SetFileSystemOptions(*fileSystem);

		for (const wxString& path: folders)
		{
			fileSystem->AddVirtualFolder(path);
		}
		Utility::Log::LogInfo("VirtualFileSystem::Convergence::BuildFileTree: file tree size -> %1", fileSystem->BuildFileTree());

		m_Convergence = std::move(fileSystem);
	}
	void MainFileSystem::InitMirroredLocations()
	{
		m_Mirrors.clear();

		for (const MirroredLocation& location: m_Manager.GetOptions().GetMirroredLocations())
		{
			if (location.ShouldUseMultiMirror())
			{
				auto fileSystem = std::make_unique<VirtualFileSystem::MultiMirror>(location.GetTarget(), location.GetSources());
				SetFileSystemOptions(*fileSystem);
				m_Mirrors.emplace_back(std::move(fileSystem));
			}
			else
			{
				auto fileSystem = std::make_unique<VirtualFileSystem::Mirror>(location.GetTarget(), location.GetSource());
				SetFileSystemOptions(*fileSystem);
				m_Mirrors.emplace_back(std::move(fileSystem));
			}
		}
	}
	void MainFileSystem::SetFileSystemOptions(VirtualFileSystem::BaseFileSystem& fileSystem)
	{
		fileSystem.EnableAsyncIO(true);
		fileSystem.EnableExtendedSecurity(true);
		fileSystem.EnableImpersonateCallerUser(false);
	}
	
	MainFileSystem::ProcessList MainFileSystem::CheckAllForRunningPrograms()
	{
		const KxUInt32Vector& runningProcesses = KxProcess::EnumProcesses();
		ProcessList activeProcesses;

		CheckForRunningPrograms(activeProcesses, runningProcesses, *m_Convergence);
		for (auto& fileSystem: m_Mirrors)
		{
			CheckForRunningPrograms(activeProcesses, runningProcesses, *fileSystem);
		}
		return activeProcesses;
	}
	void MainFileSystem::CheckForRunningPrograms(ProcessList& activeProcesses, const KxUInt32Vector& processes, BaseFileSystem& fileSystem)
	{
		for (uint32_t pid: processes)
		{
			if (fileSystem.IsProcessCreatedInVFS(pid))
			{
				activeProcesses.emplace_back(std::make_unique<KxProcess>(pid));
			}
		}
	}

	void MainFileSystem::ShowStatusDialog()
	{
		if (m_StatusDialog)
		{
			HideStatusDialog();
		}

		IMainWindow* mainWindow = IMainWindow::GetInstance();
		if (mainWindow)
		{
			mainWindow->GetFrame().Disable();
		}

		m_StatusDialog = new KxProgressDialog(&mainWindow->GetFrame(), KxID_NONE, wxEmptyString, wxDefaultPosition, wxDefaultSize, KxBTN_NONE);
		m_StatusDialog->SetCaption(IsEnabled() ? KTr("VFS.MountingCaptionDisable") : KTr("VFS.MountingCaptionEnable"));
		m_StatusDialog->SetLabel(KTr("VFS.MountingMessage"));
		m_StatusDialog->Pulse();
		m_StatusDialog->Show();
	}
	void MainFileSystem::HideStatusDialog()
	{
		if (IMainWindow* mainWindow = IMainWindow::GetInstance())
		{
			mainWindow->GetFrame().Enable();
		}

		if (m_StatusDialog)
		{
			m_StatusDialog->Destroy();
			m_StatusDialog = nullptr;
		}
	}

	bool MainFileSystem::IsOurInstance(const IVirtualFileSystem& instance) const
	{
		return ForEachInstance([&instance](IVirtualFileSystem* vfs)
		{
			return &instance == vfs;
		}) != nullptr;
	}
	size_t MainFileSystem::GetInstancesCount() const
	{
		return m_Mirrors.size() + (m_Convergence ? 1 : 0);
	}

	void MainFileSystem::OnSingleFSMounted(VirtualFSEvent& event)
	{
		if (!m_IsEnabled && IsOurInstance(event.GetFileSystem()))
		{
			m_InstancesCountEnabled++;
			if (m_InstancesCountEnabled == m_InstancesCountTotal)
			{
				OnEnabled();
			}
		}
	}
	void MainFileSystem::OnSingleFSUnmounted(VirtualFSEvent& event)
	{
		if (m_IsEnabled && m_InstancesCountEnabled != 0 && IsOurInstance(event.GetFileSystem()))
		{
			m_InstancesCountEnabled--;
			if (m_InstancesCountEnabled == 0)
			{
				OnDisabled();
			}
		}
	}

	void MainFileSystem::OnEnabled()
	{
		m_IsEnabled = true;
		BroadcastProcessor::Get().QueueEvent(VirtualFSEvent::EvtMainToggled, *this, true);
	}
	void MainFileSystem::OnDisabled()
	{
		m_IsEnabled = false;
		BroadcastProcessor::Get().QueueEvent(VirtualFSEvent::EvtMainToggled, *this, false);
	}
	
	MainFileSystem::MainFileSystem(DefaultModManager& manager)
		:m_Manager(manager)
	{
		m_BroadcastReciever.Bind(VirtualFSEvent::EvtSingleToggled, [this](VirtualFSEvent& event)
		{
			if (event.IsActivated())
			{
				OnSingleFSMounted(event);
			}
			else
			{
				OnSingleFSUnmounted(event);
			}
		});
	}
	MainFileSystem::~MainFileSystem()
	{
		Disable();
		HideStatusDialog();
	}

	bool MainFileSystem::IsEnabled() const
	{
		return m_IsEnabled;
	}
	bool MainFileSystem::Enable()
	{
		if (!m_IsEnabled)
		{
			KxCoroutine::Run([this](KxCoroutine& coroutine)
			{
				auto state = coroutine.GetNextState<State>();
				switch (state ? *state : State::ShowDialog)
				{
					case State::ShowDialog:
					{
						ShowStatusDialog();
						IVFSService::GetInstance()->EnableLog(Application::GlobalOption("VirtualFileSystem/Log").GetAttributeBool("Enabled"));

						return KxCoroutine::YieldWait(wxTimeSpan::Milliseconds(500), State::Run);
					}
					case State::Run:
					{
						// Reset counters, we'll use them to detect is all vfs instances are active.
						m_InstancesCountEnabled = 0;
						m_InstancesCountTotal = 0;

						KxStringVector nonEmptyMountPoints = CheckMountPoints();
						if (nonEmptyMountPoints.empty())
						{
							InitMainVirtualFolder();
							InitMirroredLocations();

							// Cache active instances count
							m_InstancesCountTotal = GetInstancesCount();

							// Mount all
							bool isSuccess = m_Convergence->Enable();
							if (isSuccess)
							{
								for (auto& vfs: m_Mirrors)
								{
									isSuccess = vfs->Enable();
									if (!isSuccess)
									{
										break;
									}
								}
							}

							// In case of error disable all already enabled file systems
							if (!isSuccess)
							{
								m_Convergence->Disable();
								for (auto& vfs: m_Mirrors)
								{
									vfs->Disable();
								}

								BroadcastProcessor::Get().QueueEvent(VirtualFSEvent::EvtMainToggleError, *this, false, VirtualFSEvent::Error::Unknown);
							}
						}
						else
						{
							BroadcastProcessor::Get().QueueEventEx(VirtualFSEvent::EvtMainToggleError, *this, false).On([&](VirtualFSEvent& event)
							{
								event.SetError(VirtualFSEvent::Error::NonEmptyMountPoint);
								event.SetMountPoints(std::move(nonEmptyMountPoints));
							});
						}

						return KxCoroutine::YieldWait(wxTimeSpan::Milliseconds(100), State::HideDialog);
					}
					case State::HideDialog:
					{
						HideStatusDialog();
						break;
					}
				};
				return KxCoroutine::YieldStop();
			});
			return true;
		}
		return false;
	}
	void MainFileSystem::Disable()
	{
		if (m_IsEnabled)
		{
			KxCoroutine::Run([this, state = State::ShowDialog](KxCoroutine& coroutine)
			{
				auto state = coroutine.GetNextState<State>();
				switch (state ? *state : State::ShowDialog)
				{
					case State::ShowDialog:
					{
						ShowStatusDialog();
						return KxCoroutine::YieldWait(wxTimeSpan::Milliseconds(500), State::Run);
					}
					case State::Run:
					{
						ProcessList activeProcesses = CheckAllForRunningPrograms();
						if (activeProcesses.empty())
						{
							m_Convergence->Disable();
							for (auto& vfs: m_Mirrors)
							{
								vfs->Disable();
							}
						}
						else
						{
							BroadcastProcessor::Get().QueueEvent(VirtualFSEvent::EvtMainToggleError, *this, true);

							KxTaskDialog dialog(m_StatusDialog, KxID_NONE, {}, {}, KxBTN_RETRY|KxBTN_CANCEL, KxICON_WARNING);
							dialog.SetCaption(KTr("VFS.ActiveProcesses.Caption"));
							dialog.SetMessage(KTr("VFS.ActiveProcesses.Message"));

							wxString processesList;
							for (const auto& process: activeProcesses)
							{
								// Processes created in VFS has path names if the following format:
								// \Device\Volume{<GUID>}\<Actual path name>
								// So we need to extract the path to show it to the user
								wxString path = process->GetImageName();
								wxRegEx regex(wxS("\\\\Device\\\\Volume{(.+)}\\\\(.*)"), wxRE_ADVANCED|wxRE_ICASE);
								if (regex.Matches(path))
								{
									path = regex.GetMatch(path, 2);
								}

								// If path isn't in the format above, display it as is.
								processesList += KxString::Format(wxS("\"%1\" (ID: %2)\r\n"), path, process->GetPID());
							}
							dialog.SetExMessage(processesList);
							dialog.SetOptionEnabled(KxTD_Options::KxTD_EXMESSAGE_EXPANDED);

							if (dialog.ShowModal() == KxID_RETRY)
							{
								KxAsync::DelayedCall([this]()
								{
									Disable();
								}, wxTimeSpan::Milliseconds(150));
							}
						}
						return KxCoroutine::YieldWait(wxTimeSpan::Milliseconds(100), State::HideDialog);
					}
					case State::HideDialog:
					{
						HideStatusDialog();
						break;
					}
				};
				return KxCoroutine::YieldStop();
			});
		}
	}
}
