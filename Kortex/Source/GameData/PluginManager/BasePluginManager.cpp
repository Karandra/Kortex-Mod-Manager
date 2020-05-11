#include "stdafx.h"
#include "BasePluginManager.h"
#include "Workspace.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/ProgramManager.hpp>
#include <Kortex/PluginManager.hpp>
#include "Utility/UniquePtrVector.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxProcess.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxProgressDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxComparator.h>

namespace Kortex::PluginManager
{
	void BasePluginManager::OnInit()
	{
	}
	void BasePluginManager::OnExit()
	{
	}
	void BasePluginManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		m_Config.OnLoadInstance(instance, managerNode);
	}

	intptr_t BasePluginManager::OnGetPluginPriority(const IGamePlugin& plugin) const
	{
		intptr_t priority = 0;
		intptr_t inactive = 0;

		for (const auto& currentPlugin: m_Plugins)
		{
			if (currentPlugin.get() == &plugin)
			{
				return priority - inactive;
			}

			priority++;
			if (!currentPlugin->IsActive())
			{
				inactive++;
			}
		}
		return -1;
	}
	intptr_t BasePluginManager::OnGetPluginDisplayPriority(const IGamePlugin& plugin) const
	{
		return OnGetPluginPriority(plugin);
	}

	void BasePluginManager::Invalidate()
	{
		Load();
		IWorkspace::ScheduleReloadOf<PluginManager::Workspace>();
		BroadcastProcessor::Get().ProcessEvent(PluginEvent::EvtReordered);
	}
	bool BasePluginManager::MovePlugins(const IGamePlugin::RefVector& entriesToMove, const IGamePlugin& anchor, MoveMode moveMode)
	{
		if (moveMode == MoveMode::Before)
		{
			return Utility::UniquePtrVector::MoveBefore(m_Plugins, entriesToMove, anchor);
		}
		else
		{
			return Utility::UniquePtrVector::MoveAfter(m_Plugins, entriesToMove, anchor);
		}
	}
	void BasePluginManager::SyncWithPluginsList(const KxStringVector& pluginNamesList, SyncListMode mode)
	{
		IGameProfile* profile = IGameInstance::GetActiveProfile();
		GameInstance::ProfilePlugin::Vector& pluginsList = profile->GetPlugins();
		pluginsList.clear();
		for (const wxString& name: pluginNamesList)
		{
			bool isActive = false;
			switch (mode)
			{
				case SyncListMode::ActivateAll:
				{
					isActive = true;
					break;
				}
				case SyncListMode::DeactivateAll:
				{
					isActive = false;
					break;
				}
				case SyncListMode::DoNotChange:
				{
					const IGamePlugin* plugin = FindPluginByName(name);
					isActive = plugin && plugin->IsActive();
					break;
				}
			};
			pluginsList.emplace_back(name, isActive);
		}
		profile->SaveConfig();
		Invalidate();
	}
	KxStringVector BasePluginManager::GetPluginsList(bool activeOnly) const
	{
		KxStringVector list;
		list.reserve(m_Plugins.size());

		for (const auto& entry: m_Plugins)
		{
			if (activeOnly && !entry->IsActive())
			{
				continue;
			}
			list.push_back(entry->GetName());
		}
		return list;
	}
	IGamePlugin* BasePluginManager::FindPluginByName(const wxString& name) const
	{
		auto it = std::find_if(m_Plugins.begin(), m_Plugins.end(), [&name](const auto& entry)
		{
			return KxComparator::IsEqual(name, entry->GetName(), true);
		});
		if (it != m_Plugins.end())
		{
			return it->get();
		}
		return nullptr;
	}

	bool BasePluginManager::CheckSortingTool(const PluginManager::SortingToolItem& toolItem)
	{
		if (toolItem.GetExecutable().IsEmpty() || !KxFile(toolItem.GetExecutable()).IsFileExist())
		{
			KxTaskDialog dalog(Workspace::GetInstance(), KxID_NONE, KTrf("PluginManager.Sorting.Missing.Caption", toolItem.GetName()), KTr("PluginManager.Sorting.Missing.Message"), KxBTN_OK|KxBTN_CANCEL, KxICON_WARNING);
			if (dalog.ShowModal() == KxID_OK)
			{
				KxFileBrowseDialog browseDialog(Workspace::GetInstance(), KxID_NONE, KxFBD_OPEN);
				browseDialog.AddFilter("*.exe", KTr("FileFilter.Programs"));
				if (browseDialog.ShowModal() == KxID_OK)
				{
					toolItem.SetExecutable(browseDialog.GetResult());
					return true;
				}
			}
			return false;
		}
		return true;
	}
	void BasePluginManager::RunSortingTool(const PluginManager::SortingToolItem& toolItem)
	{
		if (CheckSortingTool(toolItem))
		{
			ProgramManager::DefaultProgramItem runItem;
			runItem.SetName(toolItem.GetName());
			runItem.SetExecutable(toolItem.GetExecutable());
			runItem.SetArguments(toolItem.GetArguments());

			auto process = IProgramManager::GetInstance()->CreateProcess(runItem);
			process->SetOptionEnabled(KxPROCESS_WAIT_END, true);
			process->SetOptionEnabled(KxPROCESS_DETACHED, false);
			KxProcess& processRef = *process;

			KxProgressDialog* dialog = new KxProgressDialog(Workspace::GetInstance(), KxID_NONE, KTr("PluginManager.Sorting.Waiting.Caption"), wxDefaultPosition, wxDefaultSize, KxBTN_CANCEL);
			dialog->Bind(KxEVT_STDDIALOG_BUTTON, [&process = *process](wxNotifyEvent& event)
			{
				if (event.GetId() == KxID_CANCEL)
				{
					process.Terminate(-1, true);
				}
				event.Veto();
			});
			process->Bind(KxEVT_PROCESS_END, [this, dialog, ptr = process.release()](wxProcessEvent& event) mutable
			{
				BroadcastProcessor::Get().CallAfter([this, process = std::unique_ptr<KxProcess>(ptr)]() mutable
				{
					process.reset();
					Invalidate();
				});

				// Reload plugins order from the native source
				LoadNativeOrder();

				// Copy new order into profile plugin list
				IGameProfile::GetActive()->SyncWithCurrentState();
				Save();

				dialog->Destroy();
				IMainWindow::GetInstance()->GetFrame().Enable();
			});

			IMainWindow::GetInstance()->GetFrame().Disable();
			dialog->SetMainIcon(KxICON_INFORMATION);
			dialog->Pulse();
			dialog->Show();

			Save();
			processRef.Run(KxPROCESS_RUN_ASYNC);
		}
	}
}
