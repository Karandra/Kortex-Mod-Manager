#include "stdafx.h"
#include "VirtualFileSystem.h"
#include <Kortex/ModManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/Events.hpp>
#include "IPC/KIPCClient.h"
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
	KxStringVector VirtualFileSystem::CheckMountPoints() const
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
	void VirtualFileSystem::InitMainVirtualFolder()
	{
		KIPCClient* ipc = KIPCClient::GetInstance();

		KxStringVector folders;
		for (const IGameMod* entry: m_Manager.GetAllMods())
		{
			if (entry->IsActive())
			{
				folders.push_back(entry->GetModFilesDir());
			}
		}

		if (ipc->CreateVFS_Convergence(IGameInstance::GetActive()->GetVirtualGameDir(), m_Manager.GetOverwrites().GetModFilesDir(), folders, true))
		{
			ipc->ConvergenceVFS_SetDispatcherIndex();
		}
	}
	void VirtualFileSystem::InitMirroredLocations()
	{
		KIPCClient* ipc = KIPCClient::GetInstance();
		ipc->MirrorVFS_ClearList();

		for (const MirroredLocation& location: m_Manager.GetOptions().GetMirroredLocations())
		{
			if (location.ShouldUseMultiMirror())
			{
				ipc->CreateVFS_MultiMirror(location.GetSources(), location.GetTarget());
			}
			else
			{
				ipc->CreateVFS_Mirror(location.GetSource(), location.GetTarget());
			}
		}
	}

	void VirtualFileSystem::ShowStatusDialog()
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
	void VirtualFileSystem::HideStatusDialog()
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

	void VirtualFileSystem::OnEnabled()
	{
		m_IsEnabled = true;
	}
	void VirtualFileSystem::OnDisabled()
	{
		m_IsEnabled = false;
	}

	bool VirtualFileSystem::IsEnabled() const
	{
		return m_IsEnabled;
	}
	void VirtualFileSystem::Enable()
	{
		if (!m_IsEnabled)
		{
			IEvent::CallAfter([this]()
			{
				ShowStatusDialog();

				KxStringVector nonEmptyMountPoints = CheckMountPoints();
				if (nonEmptyMountPoints.empty())
				{
					InitMainVirtualFolder();
					InitMirroredLocations();
					KIPCClient::GetInstance()->EnableVFS();
				}
				else
				{
					m_Manager.OnMountPointsError(nonEmptyMountPoints);
				}

				HideStatusDialog();
			});
		}
	}
	void VirtualFileSystem::Disable()
	{
		if (m_IsEnabled)
		{
			IEvent::CallAfter([this]()
			{
				ShowStatusDialog();

				KIPCClient::GetInstance()->DisableVFS();

				HideStatusDialog();
			});
		}
	}
}
