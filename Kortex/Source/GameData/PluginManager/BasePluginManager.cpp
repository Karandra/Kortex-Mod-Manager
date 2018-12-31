#include "stdafx.h"
#include "BasePluginManager.h"
#include "Workspace.h"
#include <Kortex/GameInstance.hpp>
#include <Kortex/ProgramManager.hpp>
#include <Kortex/Events.hpp>
#include "UI/KWorkspace.h"
#include "Utility/KUPtrVectorUtil.h"
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
		KWorkspace::ScheduleReloadOf<PluginManager::Workspace>();
		IEvent::MakeSend<PluginManager::PluginEvent>(Events::PluginsReordered);
	}
	bool BasePluginManager::MovePlugins(const IGamePlugin::RefVector& entriesToMove, const IGamePlugin& anchor, MoveMode moveMode)
	{
		if (moveMode == MoveMode::Before)
		{
			return KUPtrVectorUtil::MoveBefore(m_Plugins, entriesToMove, anchor);
		}
		else
		{
			return KUPtrVectorUtil::MoveAfter(m_Plugins, entriesToMove, anchor);
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

	bool BasePluginManager::CheckSortingTool(const PluginManager::SortingToolEntry& entry)
	{
		if (entry.GetExecutable().IsEmpty() || !KxFile(entry.GetExecutable()).IsFileExist())
		{
			KxTaskDialog dalog(KMainWindow::GetInstance(), KxID_NONE, KTrf("PluginManager.Sorting.Missing.Caption", entry.GetName()), KTr("PluginManager.Sorting.Missing.Message"), KxBTN_OK|KxBTN_CANCEL, KxICON_WARNING);
			if (dalog.ShowModal() == KxID_OK)
			{
				KxFileBrowseDialog browseDialog(KMainWindow::GetInstance(), KxID_NONE, KxFBD_OPEN);
				browseDialog.AddFilter("*.exe", KTr("FileFilter.Programs"));
				if (browseDialog.ShowModal() == KxID_OK)
				{
					entry.SetExecutable(browseDialog.GetResult());
					return true;
				}
			}
			return false;
		}
		return true;
	}
	void BasePluginManager::RunSortingTool(const PluginManager::SortingToolEntry& entry)
	{
		if (CheckSortingTool(entry))
		{
			ProgramManager::DefaultProgramEntry runEntry;
			runEntry.SetName(entry.GetName());
			runEntry.SetExecutable(entry.GetExecutable());
			runEntry.SetArguments(entry.GetArguments());

			KxProcess* process = IProgramManager::GetInstance()->CreateProcess(runEntry).release();
			process->SetOptionEnabled(KxPROCESS_WAIT_END, true);
			process->SetOptionEnabled(KxPROCESS_DETACHED, false);

			KxProgressDialog* dialog = new KxProgressDialog(KMainWindow::GetInstance(), KxID_NONE, KTr("PluginManager.Sorting.Waiting.Caption"), wxDefaultPosition, wxDefaultSize, KxBTN_CANCEL);
			dialog->Bind(KxEVT_STDDIALOG_BUTTON, [process](wxNotifyEvent& event)
			{
				if (event.GetId() == KxID_CANCEL)
				{
					process->Terminate(-1, true);
				}
				event.Veto();
			});
			process->Bind(KxEVT_PROCESS_END, [this, process, dialog](wxProcessEvent& event)
			{
				LoadNativeOrder();
				dialog->Destroy();
				KMainWindow::GetInstance()->Show();

				IEvent::CallAfter([&process]()
				{
					delete process;
				});
			});

			KMainWindow::GetInstance()->Hide();
			dialog->SetMainIcon(KxICON_INFORMATION);
			dialog->Pulse();
			dialog->Show();

			process->Run(KxPROCESS_RUN_ASYNC);
		}
	}
}
