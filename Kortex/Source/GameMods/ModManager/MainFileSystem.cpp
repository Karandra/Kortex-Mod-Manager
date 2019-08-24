#include "stdafx.h"
#include "MainFileSystem.h"
#include <Kortex/ModManager.hpp>
#include <Kortex/GameInstance.hpp>
#include "VirtualFileSystem/Mirror.h"
#include "VirtualFileSystem/MultiMirror.h"
#include "VirtualFileSystem/Convergence.h"
#include "VirtualFileSystem/VirtualFSEvent.h"
#include "DefaultModManager.h"
#include "UI/KMainWindow.h"
#include "Utility/Log.h"
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxProgressDialog.h>
#include <Kx/Async/Coroutine.h>

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

	void MainFileSystem::ShowStatusDialog()
	{
		if (m_StatusDialog)
		{
			HideStatusDialog();
		}

		KMainWindow* mainWindow = KMainWindow::GetInstance();
		if (mainWindow)
		{
			mainWindow->Disable();
		}

		m_StatusDialog = new KxProgressDialog(mainWindow, KxID_NONE, wxEmptyString, wxDefaultPosition, wxDefaultSize, KxBTN_NONE);
		m_StatusDialog->SetCaption(IsEnabled() ? KTr("VFS.MountingCaptionDisable") : KTr("VFS.MountingCaptionEnable"));
		m_StatusDialog->SetLabel(KTr("VFS.MountingMessage"));
		m_StatusDialog->Pulse();
		m_StatusDialog->Show();
	}
	void MainFileSystem::HideStatusDialog()
	{
		if (KMainWindow* mainWindow = KMainWindow::GetInstance())
		{
			mainWindow->Enable();
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
	void MainFileSystem::Enable()
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
							m_Convergence->Enable();
							for (auto& vfs: m_Mirrors)
							{
								vfs->Enable();
							}
						}
						else
						{
							m_Manager.OnMountPointError(nonEmptyMountPoints);
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
						m_Convergence->Disable();
						for (auto& vfs: m_Mirrors)
						{
							vfs->Disable();
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
