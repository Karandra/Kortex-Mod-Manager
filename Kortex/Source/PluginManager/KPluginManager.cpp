#include "stdafx.h"
#include "KPluginManager.h"
#include "KPluginManagerBethesdaGeneric.h"
#include "KPluginManagerBethesdaGeneric2.h"
#include "KPluginManagerBethesdaMorrowind.h"
#include "KPluginManagerWorkspace.h"
#include "KPMPluginReader.h"
#include "ModManager/KModManager.h"
#include "ModManager/KModManagerDispatcher.h"
#include "Profile/KPluginManagerConfig.h"
#include "RunManager/KRunManager.h"
#include "UI/KWorkspace.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxProcess.h>
#include <KxFramework/KxProgressDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>

KxSingletonPtr_Define(KPluginManager);

KPluginManager* KPluginManager::QueryInterface(const wxString& interfaceName, const KxXMLNode& configNode, const KPluginManagerConfig* profilePluginConfig)
{
	if (!HasInstance() && configNode.IsOK())
	{
		if (interfaceName == "BethesdaGeneric")
		{
			new KPluginManagerBethesdaGeneric(interfaceName, configNode, profilePluginConfig);
		}
		else if (interfaceName == "BethesdaGeneric2")
		{
			new KPluginManagerBethesdaGeneric2(interfaceName, configNode, profilePluginConfig);
		}
		else if (interfaceName == "BethesdaMorrowind")
		{
			new KPluginManagerBethesdaMorrowind(interfaceName, configNode, profilePluginConfig);
		}
		else
		{
			KLogMessage("KPluginManager::QueryInterface: Unknown interface requested \"%s\"", interfaceName);
		}
	}
	return GetInstance();
}

KPMPluginEntryRefVector KPluginManager::CollectDependentPlugins(const KPMPluginEntry* pluginEntry, bool bFirstOnly) const
{
	KPMPluginEntryRefVector tDependentList;
	if (bFirstOnly)
	{
		tDependentList.reserve(1);
	}

	const wxString nameL = KxString::ToLower(pluginEntry->GetName());
	for (auto& entry: GetEntries())
	{
		if (entry->IsEnabled())
		{
			if (KPMPluginReader* reader = entry->GetPluginReader())
			{
				KxStringVector dependenciesList = reader->GetDependencies();
				auto it = std::find_if(dependenciesList.begin(), dependenciesList.end(), [&nameL](const wxString& sDepName)
				{
					return nameL == KxString::ToLower(sDepName);
				});
				if (it != dependenciesList.end())
				{
					tDependentList.push_back(entry.get());
					if (bFirstOnly)
					{
						break;
					}
				}
			}
		}
	}

	return tDependentList;
}

KPluginManager::KPluginManager(const wxString& interfaceName, const KxXMLNode& configNode, const KPluginManagerConfig* profilePluginConfig)
	:m_GeneralOptions(this, "General"), m_SortingToolsOptions(this, "SortingTools")
{
}
KPluginManager::~KPluginManager()
{
}

wxString KPluginManager::GetID() const
{
	return "KPluginManager";
}
wxString KPluginManager::GetName() const
{
	return T("ToolBar.PluginManager");
}
wxString KPluginManager::GetVersion() const
{
	return "1.2.1";
}

KPMPluginEntry* KPluginManager::NewPluginEntry(const wxString& name, bool isActive, KPMPluginEntryType type)
{
	return new KPMPluginEntry(name, isActive, type);
}

bool KPluginManager::HasDependentPlugins(const KPMPluginEntry* pluginEntry) const
{
	auto list = CollectDependentPlugins(pluginEntry, true);
	return !list.empty() && list.front()->IsEnabled();
}
KPMPluginEntryRefVector KPluginManager::GetDependentPlugins(const KPMPluginEntry* pluginEntry) const
{
	return CollectDependentPlugins(pluginEntry, false);
}
const KModEntry* KPluginManager::GetParentMod(const KPMPluginEntry* pluginEntry) const
{
	return NULL;
}
bool KPluginManager::IsPluginActive(const wxString& sPluginName) const
{
	const KPMPluginEntry* entry = FindPluginByName(sPluginName);
	return entry ? entry->IsEnabled() : false;
}
void KPluginManager::SyncWithPluginsList(const KxStringVector& tPluginNamesList, SyncListMode mode)
{
	KModList::PluginEntryVector& list = KModManager::GetListManager().GetCurrentList().GetPlugins();
	list.clear();
	for (const wxString& name: tPluginNamesList)
	{
		bool isEnabled = false;
		switch (mode)
		{
			case EnableAll:
			{
				isEnabled = true;
				break;
			}
			case DisableAll:
			{
				isEnabled = false;
				break;
			}
			case DoNotChange:
			{
				const KPMPluginEntry* pPlugin = FindPluginByName(name);
				isEnabled = pPlugin && pPlugin->IsEnabled();
				break;
			}
		};
		list.emplace_back(name, isEnabled);
	}
	KModManager::GetListManager().SaveLists();
}
KxStringVector KPluginManager::GetPluginsList(bool bActiveOnly) const
{
	KxStringVector list;
	list.reserve(GetEntries().size());

	for (const auto& entry: GetEntries())
	{
		if (bActiveOnly && !entry->IsEnabled())
		{
			continue;
		}

		list.push_back(entry->GetName());
	}
	return list;
}
KPMPluginEntry* KPluginManager::FindPluginByName(const wxString& name) const
{
	wxString nameL = KxString::ToLower(name);
	auto it = std::find_if(GetEntries().cbegin(), GetEntries().cend(), [&nameL](const auto& entry)
	{
		return KxString::ToLower(entry->GetName()) == nameL;
	});
	if (it != GetEntries().cend())
	{
		return it->get();
	}
	return NULL;
}
void KPluginManager::UpdateAllPlugins()
{
	for (auto& entry: GetEntries())
	{
		entry->OnUpdate();
	}
}

wxString KPluginManager::GetPluginTypeName(KPMPluginEntryType type) const
{
	if (type & KPMPE_TYPE_NORMAL && type & KPMPE_TYPE_LIGHT)
	{
		return wxString::Format("%s (%s)", T("PluginManager.PluginType.Normal"), T("PluginManager.PluginType.Light"));
	}
	if (type & KPMPE_TYPE_MASTER && type & KPMPE_TYPE_LIGHT)
	{
		return wxString::Format("%s (%s)", T("PluginManager.PluginType.Master"), T("PluginManager.PluginType.Light"));
	}

	if (type & KPMPE_TYPE_MASTER)
	{
		return T("PluginManager.PluginType.Master");
	}
	if (type & KPMPE_TYPE_NORMAL)
	{
		return T("PluginManager.PluginType.Normal");
	}

	return wxEmptyString;
}
bool KPluginManager::IsValidModIndex(int nModIndex) const
{
	return nModIndex >= 0 && (size_t)nModIndex < GetEntries().size();
}
int KPluginManager::GetPluginIndex(const KPMPluginEntry* modEntry) const
{
	auto it = std::find_if(GetEntries().cbegin(), GetEntries().cend(), [modEntry](const auto& v)
	{
		return v.get() == modEntry;
	});
	if (it != GetEntries().cend())
	{
		return std::distance(GetEntries().cbegin(), it);
	}
	return wxNOT_FOUND;
}
bool KPluginManager::MovePluginsIntoThis(const KPMPluginEntryRefVector& entriesToMove, const KPMPluginEntry* pAnchor)
{
	auto Compare = [pAnchor](const KPMPluginEntryVector::value_type& entry)
	{
		return entry.get() == pAnchor;
	};

	// Check if anchor is not one of moved elements
	if (std::find(entriesToMove.begin(), entriesToMove.end(), pAnchor) != entriesToMove.end())
	{
		return false;
	}

	auto it = std::find_if(GetEntries().begin(), GetEntries().end(), Compare);
	if (it != GetEntries().end())
	{
		// Remove from existing place
		GetEntries().erase(std::remove_if(GetEntries().begin(), GetEntries().end(), [&entriesToMove](KPMPluginEntryVector::value_type& entry)
		{
			// Release unique_ptr's and remove them
			if (std::find(entriesToMove.begin(), entriesToMove.end(), entry.get()) != entriesToMove.end())
			{
				entry.release();
				return true;
			}
			return false;
		}), GetEntries().end());

		// Iterator may have been invalidated
		it = std::find_if(GetEntries().begin(), GetEntries().end(), Compare);
		if (it != GetEntries().end())
		{
			// Insert after anchor
			size_t index = 1;
			for (auto i = entriesToMove.begin(); i != entriesToMove.end(); ++i)
			{
				GetEntries().emplace(it + index, *i);
				index++;
			}
			return true;
		}
	}
	return false;
}
void KPluginManager::SetAllPluginsEnabled(bool isEnabled)
{
	for (auto& entry: GetEntries())
	{
		entry->SetEnabled(isEnabled);
	}
}

bool KPluginManager::CheckSortingTool(const KPluginManagerConfigSortingToolEntry& entry)
{
	if (entry.GetExecutable().IsEmpty() || !KxFile(entry.GetExecutable()).IsFileExist())
	{
		KxTaskDialog tDalog(KApp::Get().GetMainWindow(), KxID_NONE, T("PluginManager.Sorting.Missing.Caption", entry.GetName()), T("PluginManager.Sorting.Missing.Message"), KxBTN_OK|KxBTN_CANCEL, KxICON_WARNING);
		if (tDalog.ShowModal() == KxID_OK)
		{
			KxFileBrowseDialog browseDialog(KApp::Get().GetMainWindow(), KxID_NONE, KxFBD_OPEN);
			browseDialog.AddFilter("*.exe", T("FileFilter.Programs"));
			if (browseDialog.ShowModal() == KxID_OK)
			{
				entry.SetExecutable(browseDialog.GetResult());
				KApp::Get().SaveSettings();
				return true;
			}
		}
		return false;
	}
	return true;
}
void KPluginManager::RunSortingTool(const KPluginManagerConfigSortingToolEntry& entry)
{
	if (CheckSortingTool(entry))
	{
		KRunManagerProgram runEntry;
		runEntry.SetName(entry.GetName());
		runEntry.SetExecutable(entry.GetExecutable());
		runEntry.SetArguments(entry.GetArguments());

		KxProgressDialog* dialog = new KxProgressDialog(KApp::Get().GetMainWindow(), KxID_NONE, T("PluginManager.Sorting.Waiting.Caption"), wxDefaultPosition, wxDefaultSize, KxBTN_CANCEL);
		KxProcess* process = KRunManager::Get().RunEntryDelayed(runEntry, dialog);

		dialog->Bind(KxEVT_STDDIALOG_BUTTON, [process](wxNotifyEvent& event)
		{
			if (event.GetId() == KxID_CANCEL)
			{
				process->Terminate(-1, true);
			}
			event.Veto();
		});
		process->Bind(wxEVT_END_PROCESS, [this, process, dialog](wxProcessEvent& event)
		{
			LoadNativeOrder();
			KApp::Get().GetMainWindow()->Show();
			event.Skip();
		});

		KApp::Get().GetMainWindow()->Hide();
		dialog->SetMainIcon(KxICON_INFORMATION);
		dialog->Pulse();
		dialog->Show();

		process->Run(KxPROCESS_RUN_ASYNC);
	}
}
