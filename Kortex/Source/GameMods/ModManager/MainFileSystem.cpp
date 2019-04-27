#include "stdafx.h"
#include "MainFileSystem.h"
#include <Kortex/ModManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/Events.hpp>
#include "VirtualFileSystem/Mirror.h"
#include "VirtualFileSystem/MultiMirror.h"
#include "VirtualFileSystem/Convergence.h"
#include "Utility/Log.h"
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxProgressDialog.h>

namespace
{
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
		for (const IGameMod* entry: m_Manager.GetAllMods(true, true))
		{
			folders.push_back(entry->GetModFilesDir());
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
		fileSystem.EnableImpersonateCallerUser(true);
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

	void MainFileSystem::OnVFSMounted(VFSEvent& event)
	{
		if (!m_IsEnabled && event.GetFileSystem() && IsOurInstance(*event.GetFileSystem()))
		{
			m_InstancesCountEnabled++;
			if (m_InstancesCountEnabled == m_InstancesCountTotal)
			{
				OnEnabled();
			}
		}
	}
	void MainFileSystem::OnVFSUnmounted(VFSEvent& event)
	{
		if (m_IsEnabled && m_InstancesCountEnabled != 0 && event.GetFileSystem() && IsOurInstance(*event.GetFileSystem()))
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
		IEvent::MakeQueue<VFSEvent>(Events::MainVFSToggled, *this, true);
	}
	void MainFileSystem::OnDisabled()
	{
		m_IsEnabled = false;
		IEvent::MakeQueue<VFSEvent>(Events::MainVFSToggled, *this, false);
	}
	
	MainFileSystem::MainFileSystem(DefaultModManager& manager)
		:m_Manager(manager)
	{
		IEvent::Bind(Events::SingleVFSToggled, &MainFileSystem::OnVFSMounted, this);
		IEvent::Bind(Events::SingleVFSToggled, &MainFileSystem::OnVFSUnmounted, this);
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
			ShowStatusDialog();

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

			HideStatusDialog();
		}
	}
	void MainFileSystem::Disable()
	{
		if (m_IsEnabled)
		{
			ShowStatusDialog();

			m_Convergence->Disable();
			for (auto& vfs: m_Mirrors)
			{
				vfs->Disable();
			}

			HideStatusDialog();
		}
	}
}
