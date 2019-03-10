#include "stdafx.h"
#include "VFSActivator.h"
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
	KxStringVector VFSActivator::CheckMountPoints() const
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
	void VFSActivator::InitMainVirtualFolder()
	{
		m_Convergence.reset();

		KxStringVector folders;
		for (const IGameMod* entry: m_Manager.GetAllMods(true, true))
		{
			folders.push_back(entry->GetModFilesDir());
		}

		const IGameInstance* instance = IGameInstance::GetActive();
		auto vfs = std::make_unique<VirtualFileSystem::Convergence>(instance->GetVirtualGameDir(), m_Manager.GetOverwrites().GetModFilesDir());
		vfs->EnableINIOptimization(true);
		vfs->EnableSecurityFunctions(true);
		for (const wxString& path: folders)
		{
			vfs->AddVirtualFolder(path);
		}

		size_t indexSize = vfs->BuildDispatcherIndex();
		Utility::Log::LogInfo("VirtualFileSystem::Convergence::BuildDispatcherIndex: index size -> %1", indexSize);

		m_Convergence = std::move(vfs);
	}
	void VFSActivator::InitMirroredLocations()
	{
		m_Mirrors.clear();

		for (const MirroredLocation& location: m_Manager.GetOptions().GetMirroredLocations())
		{
			if (location.ShouldUseMultiMirror())
			{
				m_Mirrors.emplace_back(std::make_unique<VirtualFileSystem::MultiMirror>(location.GetTarget(), location.GetSources()));
			}
			else
			{
				m_Mirrors.emplace_back(std::make_unique<VirtualFileSystem::Mirror>(location.GetTarget(), location.GetSource()));
			}
		}
	}

	void VFSActivator::ShowStatusDialog()
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
	void VFSActivator::HideStatusDialog()
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

	bool VFSActivator::IsOurInstance(const IVirtualFileSystem& instance) const
	{
		return ForEachInstance([&instance](IVirtualFileSystem* vfs)
		{
			return &instance == vfs;
		}) != nullptr;
	}
	size_t VFSActivator::GetInstancesCount() const
	{
		return m_Mirrors.size() + (m_Convergence ? 1 : 0);
	}

	void VFSActivator::OnVFSMounted(VFSEvent& event)
	{
		if (!m_IsEnabled && event.GetVFS() && IsOurInstance(*event.GetVFS()))
		{
			m_InstancesCountEnabled++;
			if (m_InstancesCountEnabled == m_InstancesCountTotal)
			{
				OnEnabled();
			}
		}
	}
	void VFSActivator::OnVFSUnmounted(VFSEvent& event)
	{
		if (m_IsEnabled && m_InstancesCountEnabled != 0 && event.GetVFS() && IsOurInstance(*event.GetVFS()))
		{
			m_InstancesCountEnabled--;
			if (m_InstancesCountEnabled == 0)
			{
				OnDisabled();
			}
		}
	}

	void VFSActivator::OnEnabled()
	{
		m_IsEnabled = true;
		IEvent::MakeQueue<VFSEvent>(Events::MainVFSToggled, *this, true);
	}
	void VFSActivator::OnDisabled()
	{
		m_IsEnabled = false;
		IEvent::MakeQueue<VFSEvent>(Events::MainVFSToggled, *this, false);
	}
	
	VFSActivator::VFSActivator(DefaultModManager& manager)
		:m_Manager(manager)
	{
		IEvent::Bind(Events::SingleVFSToggled, &VFSActivator::OnVFSMounted, this);
		IEvent::Bind(Events::SingleVFSToggled, &VFSActivator::OnVFSUnmounted, this);
	}
	VFSActivator::~VFSActivator()
	{
		Disable();
		HideStatusDialog();
	}

	bool VFSActivator::IsEnabled() const
	{
		return m_IsEnabled;
	}
	void VFSActivator::Enable()
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
				m_Manager.OnMountPointsError(nonEmptyMountPoints);
			}

			HideStatusDialog();
		}
	}
	void VFSActivator::Disable()
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
