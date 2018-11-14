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
#include "KUPtrVectorUtil.h"

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
	KEvent::Bind(KEVT_PROFILE_SELECTED, &KPluginManager::OnVirtualTreeInvalidated, this);

	KEvent::Bind(KEVT_MODS_REORDERED, &KPluginManager::OnVirtualTreeInvalidated, this);
	KEvent::Bind(KEVT_MOD_TOGGLED, &KPluginManager::OnVirtualTreeInvalidated, this);
	KEvent::Bind(KEVT_MOD_FILES_CHANGED, &KPluginManager::OnVirtualTreeInvalidated, this);
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
	Invalidate();
}

KPluginManager::KPluginManager(const wxString& interfaceName, const KxXMLNode& configNode)
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
	return KTr("PluginManager.Name");
}
wxString KPluginManager::GetVersion() const
{
	return "1.3";
}

void KPluginManager::Invalidate()
{
	Load();
	KWorkspace::ScheduleReloadOf<KPluginManagerWorkspace>();
	KEvent::MakeSend<KPluginEvent>(KEVT_PLUGINS_REORDERED);
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

bool KPluginManager::MovePluginsIntoThis(const KPluginEntry::RefVector& entriesToMove, const KPluginEntry& anchor, MoveMode moveMode)
{
	if (moveMode == MoveMode::Before)
	{
		return KUPtrVectorUtil::MoveBefore(m_Entries, entriesToMove, anchor);
	}
	else
	{
		return KUPtrVectorUtil::MoveAfter(m_Entries, entriesToMove, anchor);
	}
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
	KProfile* profile = KGameInstance::GetActiveProfile();
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
	Invalidate();
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
		KxTaskDialog dalog(KMainWindow::GetInstance(), KxID_NONE, KTrf("PluginManager.Sorting.Missing.Caption", entry.GetName()), KTr("PluginManager.Sorting.Missing.Message"), KxBTN_OK|KxBTN_CANCEL, KxICON_WARNING);
		if (dalog.ShowModal() == KxID_OK)
		{
			KxFileBrowseDialog browseDialog(KMainWindow::GetInstance(), KxID_NONE, KxFBD_OPEN);
			browseDialog.AddFilter("*.exe", KTr("FileFilter.Programs"));
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

		KxProcess& process = KProgramManager::GetInstance()->CreateProcess(runEntry);
		process.SetOptionEnabled(KxPROCESS_WAIT_END, true);
		process.SetOptionEnabled(KxPROCESS_DETACHED, false);

		KxProgressDialog* dialog = new KxProgressDialog(KMainWindow::GetInstance(), KxID_NONE, KTr("PluginManager.Sorting.Waiting.Caption"), wxDefaultPosition, wxDefaultSize, KxBTN_CANCEL);
		dialog->Bind(KxEVT_STDDIALOG_BUTTON, [&process](wxNotifyEvent& event)
		{
			if (event.GetId() == KxID_CANCEL)
			{
				process.Terminate(-1, true);
			}
			event.Veto();
		});
		process.Bind(KxEVT_PROCESS_END, [this, &process, dialog](wxProcessEvent& event)
		{
			LoadNativeOrder();
			dialog->Destroy();
			KMainWindow::GetInstance()->Show();

			KEvent::CallAfter([&process]()
			{
				KProgramManager::GetInstance()->DestroyProcess(process);
			});
		});

		KMainWindow::GetInstance()->Hide();
		dialog->SetMainIcon(KxICON_INFORMATION);
		dialog->Pulse();
		dialog->Show();

		process.Run(KxPROCESS_RUN_ASYNC);
	}
}
