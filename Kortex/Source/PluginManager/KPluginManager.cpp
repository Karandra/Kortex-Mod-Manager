#include "stdafx.h"
#include "KPluginManager.h"
#include "KPluginManagerWorkspace.h"

#include "KPluginManagerBethesda.h"
#include "KPluginManagerBethesda2.h"
#include "KPluginManagerBethesdaMW.h"

#include "KPluginReaderBethesdaMorrowind.h"
#include "KPluginReaderBethesdaOblivion.h"
#include "KPluginReaderBethesdaSkyrim.h"

#include "ModManager/KModManager.h"
#include "ModManager/KDispatcher.h"
#include "GameInstance/KGameInstance.h"
#include "Profile/KProfile.h"
#include "GameInstance/Config/KPluginManagerConfig.h"
#include "ProgramManager/KProgramManager.h"
#include "UI/KWorkspace.h"
#include "KEvents.h"

#include <KxFramework/KxProcess.h>
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxProgressDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>

std::unique_ptr<KPluginManager> KPluginManager::QueryInterface(const wxString& interfaceName, const KxXMLNode& configNode)
{
	if (!HasInstance())
	{
		if (interfaceName == KPLUGIN_IMANAGER_BETHESDA)
		{
			return std::make_unique<KPluginManagerBethesda>(interfaceName, configNode);
		}
		else if (interfaceName == KPLUGIN_IMANAGER_BETHESDA2)
		{
			return std::make_unique<KPluginManagerBethesda2>(interfaceName, configNode);
		}
		else if (interfaceName == KPLUGIN_IMANAGER_BETHESDAMW)
		{
			return std::make_unique<KPluginManagerBethesdaMW>(interfaceName, configNode);
		}
		else
		{
			KLogMessage("KPluginManager::QueryInterface: Unknown interface requested \"%s\"", interfaceName);
		}
	}
	return NULL;
}
std::unique_ptr<KPluginReader> KPluginManager::QueryPluginReader(const wxString& formatName)
{
	if (formatName == KPLUGIN_IFILE_BETHESDA_MORROWIND)
	{
		return std::make_unique<KPluginReaderBethesdaMorrowind>();
	}
	else if (formatName == KPLUGIN_IFILE_BETHESDA_OBLIVION)
	{
		return std::make_unique<KPluginReaderBethesdaOblivion>();
	}
	else if (formatName == KPLUGIN_IFILE_BETHESDA_SKYRIM)
	{
		return std::make_unique<KPluginReaderBethesdaSkyrim>();
	}
	return NULL;
}

void KPluginManager::OnInit()
{
}
void KPluginManager::ReadPluginsData()
{
	for (auto& pluginEntry: m_Entries)
	{
		pluginEntry->ReadPluginData();
	}
}

void KPluginManager::OnVirtualTreeInvalidated(KEvent& event)
{
	Load();
	KWorkspace::ScheduleReloadOf<KPluginManagerWorkspace>();
}

KPluginManager::KPluginManager(const wxString& interfaceName, const KxXMLNode& configNode)
	:m_GeneralOptions(this, "General"), m_SortingToolsOptions(this, "SortingTools")
{
	KEvent::Bind(KEVT_MOD_VIRTUAL_TREE_INVALIDATED, &KPluginManager::OnVirtualTreeInvalidated, this);
	KEvent::Bind(KEVT_MOD_TOGGLED, &KPluginManager::OnVirtualTreeInvalidated, this);
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
	return T("PluginManager.Name");
}
wxString KPluginManager::GetVersion() const
{
	return "1.3";
}

bool KPluginManager::IsValidModIndex(intptr_t modIndex) const
{
	return modIndex >= 0 && (size_t)modIndex < m_Entries.size();
}
intptr_t KPluginManager::GetPluginOrderIndex(const KPluginEntry& modEntry) const
{
	auto it = std::find_if(m_Entries.begin(), m_Entries.end(), [&modEntry](const auto& v)
	{
		return v.get() == &modEntry;
	});
	if (it != m_Entries.end())
	{
		return std::distance(m_Entries.begin(), it);
	}
	return wxNOT_FOUND;
}

bool KPluginManager::MovePluginsIntoThis(const KPluginEntry::RefVector& entriesToMove, const KPluginEntry& anchor)
{
	auto Compare = [&anchor](const auto& entry)
	{
		return entry.get() == &anchor;
	};

	// Check if anchor is not one of moved elements
	if (std::find(entriesToMove.begin(), entriesToMove.end(), &anchor) != entriesToMove.end())
	{
		return false;
	}

	auto it = std::find_if(m_Entries.begin(), m_Entries.end(), Compare);
	if (it != m_Entries.end())
	{
		// Remove from existing place
		m_Entries.erase(std::remove_if(m_Entries.begin(), m_Entries.end(), [&entriesToMove](auto& entry)
		{
			// Release unique_ptr's and remove them
			if (std::find(entriesToMove.begin(), entriesToMove.end(), entry.get()) != entriesToMove.end())
			{
				entry.release();
				return true;
			}
			return false;
		}), m_Entries.end());

		// Iterator may have been invalidated
		it = std::find_if(m_Entries.begin(), m_Entries.end(), Compare);
		if (it != m_Entries.end())
		{
			// Insert after anchor
			size_t index = 1;
			for (auto i = entriesToMove.begin(); i != entriesToMove.end(); ++i)
			{
				m_Entries.emplace(it + index, *i);
				index++;
			}
			return true;
		}
	}
	return false;
}
void KPluginManager::SetAllPluginsEnabled(bool isEnabled)
{
	for (auto& entry: m_Entries)
	{
		entry->SetEnabled(isEnabled);
	}
}

bool KPluginManager::IsPluginActive(const wxString& pluginName) const
{
	const KPluginEntry* entry = FindPluginByName(pluginName);
	return entry ? entry->IsEnabled() : false;
}
void KPluginManager::SyncWithPluginsList(const KxStringVector& pluginNamesList, SyncListMode mode)
{
	KProfile* profile = KGameInstance::GetCurrentProfile();
	KProfilePlugin::Vector& pluginsList = profile->GetPlugins();
	pluginsList.clear();
	for (const wxString& name: pluginNamesList)
	{
		bool isEnabled = false;
		switch (mode)
		{
			case SyncListMode::EnableAll:
			{
				isEnabled = true;
				break;
			}
			case SyncListMode::DisableAll:
			{
				isEnabled = false;
				break;
			}
			case SyncListMode::DoNotChange:
			{
				const KPluginEntry* plugin = FindPluginByName(name);
				isEnabled = plugin && plugin->IsEnabled();
				break;
			}
		};
		pluginsList.emplace_back(name, isEnabled);
	}
	profile->Save();

	// Reload to update internal state
	Load();
}
KxStringVector KPluginManager::GetPluginsList(bool activeOnly) const
{
	KxStringVector list;
	list.reserve(m_Entries.size());

	for (const auto& entry: m_Entries)
	{
		if (activeOnly && !entry->IsEnabled())
		{
			continue;
		}

		list.push_back(entry->GetName());
	}
	return list;
}
KPluginEntry* KPluginManager::FindPluginByName(const wxString& name) const
{
	auto it = std::find_if(m_Entries.begin(), m_Entries.end(), [&name](const auto& entry)
	{
		return KxComparator::IsEqual(name, entry->GetName(), true);
	});
	if (it != m_Entries.end())
	{
		return it->get();
	}
	return NULL;
}

bool KPluginManager::CheckSortingTool(const KPluginManagerConfigSortingToolEntry& entry)
{
	if (entry.GetExecutable().IsEmpty() || !KxFile(entry.GetExecutable()).IsFileExist())
	{
		KxTaskDialog dalog(KMainWindow::GetInstance(), KxID_NONE, TF("PluginManager.Sorting.Missing.Caption").arg(entry.GetName()), T("PluginManager.Sorting.Missing.Message"), KxBTN_OK|KxBTN_CANCEL, KxICON_WARNING);
		if (dalog.ShowModal() == KxID_OK)
		{
			KxFileBrowseDialog browseDialog(KMainWindow::GetInstance(), KxID_NONE, KxFBD_OPEN);
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
		KProgramEntry runEntry;
		runEntry.SetName(entry.GetName());
		runEntry.SetExecutable(entry.GetExecutable());
		runEntry.SetArguments(entry.GetArguments());

		KxProgressDialog* dialog = new KxProgressDialog(KMainWindow::GetInstance(), KxID_NONE, T("PluginManager.Sorting.Waiting.Caption"), wxDefaultPosition, wxDefaultSize, KxBTN_CANCEL);
		KxProcess* process = KProgramManager::GetInstance()->RunEntryDelayed(runEntry, dialog);

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
			KMainWindow::GetInstance()->Show();
			event.Skip();
		});

		KMainWindow::GetInstance()->Hide();
		dialog->SetMainIcon(KxICON_INFORMATION);
		dialog->Pulse();
		dialog->Show();

		process->Run(KxPROCESS_RUN_ASYNC);
	}
}
