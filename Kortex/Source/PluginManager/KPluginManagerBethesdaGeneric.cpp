#include "stdafx.h"
#include "KVariablesDatabase.h"
#include "KPluginManagerBethesdaGeneric.h"
#include "KPluginManagerWorkspace.h"
#include "KPluginManager.h"
#include "KPMPluginReader.h"
#include "UI/KWorkspace.h"
#include "UI/KWorkspaceController.h"
#include "ModManager/KModManager.h"
#include "ModManager/KModEntry.h"
#include "RunManager/KRunManager.h"
#include "ModManager/KModManagerDispatcher.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxProcess.h>
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxProgressDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>

void KPMPluginEntryBethesdaGeneric::UpdateHasDependentPlugins()
{
	m_HasDependentPlugins = KPluginManager::GetInstance()->HasDependentPlugins(this);
}

KPMPluginEntryBethesdaGeneric::KPMPluginEntryBethesdaGeneric(const wxString& name, bool isActive, KPMPluginEntryType type)
	:KPMPluginEntry(name, isActive, type)
{
	//UpdateHasDependentPlugins();
}

void KPMPluginEntryBethesdaGeneric::OnUpdate()
{
	//UpdateHasDependentPlugins();
	KPMPluginEntry::OnUpdate();
}

bool KPMPluginEntryBethesdaGeneric::CanToggleEnabled() const
{
	//return !m_HasDependentPlugins;
	return KPMPluginEntry::CanToggleEnabled();
}
bool KPMPluginEntryBethesdaGeneric::IsEnabled() const
{
	//return m_HasDependentPlugins || KPMPluginEntry::IsEnabled();
	return KPMPluginEntry::IsEnabled();
}

//////////////////////////////////////////////////////////////////////////
void KPluginManagerBethesdaGeneric::Clear()
{
	m_Entries.clear();
}
void KPluginManagerBethesdaGeneric::SortByDate()
{
	std::sort(GetEntries().begin(), GetEntries().end(), [this](const auto& pEntry1, const auto& pEntry2)
	{
		wxFileName tFile1(pEntry1->GetFullPath());
		wxFileName tFile2(pEntry2->GetFullPath());

		return tFile1.GetModificationTime() < tFile2.GetModificationTime();
	});
}

KPMPluginEntryType KPluginManagerBethesdaGeneric::GetPluginTypeFromPath(const wxString& name) const
{
	wxString ext = name.AfterLast('.');
	if (ext.IsSameAs("esm", false))
	{
		return KPMPE_TYPE_MASTER;
	}
	else if (ext.IsSameAs("esp", false))
	{
		return KPMPE_TYPE_NORMAL;
	}
	return KPMPE_TYPE_INVALID;
}
void KPluginManagerBethesdaGeneric::SetParentModAndPluginFile(KPMPluginEntry* entry)
{
	entry->SetParentMod(GetParentMod(entry));
	
	if (KPMPluginReader* reader = entry->GetPluginReader())
	{
		reader->Create(entry->GetFullPath());
	}
}

wxString KPluginManagerBethesdaGeneric::OnWriteToLoadOrder(const KPMPluginEntry* entry) const
{
	return entry->GetName();
}
wxString KPluginManagerBethesdaGeneric::OnWriteToActiveOrder(const KPMPluginEntry* entry) const
{
	return entry->GetName();
}
KWorkspace* KPluginManagerBethesdaGeneric::CreateWorkspace(KMainWindow* mainWindow)
{
	m_Workspace = new KPluginManagerWorkspace(mainWindow, this);
	return m_Workspace;
}

void KPluginManagerBethesdaGeneric::LoadNativeOrderBG()
{
	Clear();
	auto files = KModManager::GetDispatcher().FindFiles(m_PluginsLocation, KxFile::NullFilter, KxFS_FILE, false);
	KModList& tLoadOrder = KModManager::GetListManager().GetCurrentList();

	// Load from 'LoadOrder.txt'
	for (const wxString& name: KxTextFile::ReadToArray(m_ActiveListFile))
	{
		// Find whether plugin with this name exist
		wxString nameL = KxString::ToLower(name);
		auto it = std::find_if(files.begin(), files.end(), [&nameL](const KxFileFinderItem& item)
		{
			return KxString::ToLower(item.GetName()) == nameL;
		});

		if (!nameL.StartsWith('#') && it != files.end())
		{
			KPMPluginEntryType type = GetPluginTypeFromPath(nameL);
			if (type != KPMPE_TYPE_INVALID)
			{
				KPMPluginEntry* entry = EmplacePluginEntry(name, false, type);
				entry->SetFullPath(it->GetFullPath());
				SetParentModAndPluginFile(entry);
			}
		}
	}

	// Load new plugins from folder
	for (const KxFileFinderItem& item: files)
	{
		if (!FindPluginByName(item.GetName()))
		{
			KPMPluginEntry* entry = EmplacePluginEntry(item.GetName(), false, GetPluginTypeFromPath(item.GetName()));
			entry->SetFullPath(item.GetFullPath());
			SetParentModAndPluginFile(entry);
		}
	}

	if (ShouldSortByFileModificationDate())
	{
		SortByDate();
	}
	LoadNativeActiveBG();

	// Enable all std-content
	for (auto& entry: GetEntries())
	{
		if (entry->GetStdContentEntry())
		{
			entry->SetEnabled(true);
		}
	}
}
void KPluginManagerBethesdaGeneric::LoadNativeActiveBG()
{
	// Load names from 'Plugins.txt' it they are not already added.
	// Activate all new added and existing items with same name.
	for (const wxString& name: KxTextFile::ReadToArray(m_ActiveListFile))
	{
		if (KPMPluginEntry* entry = FindPluginByName(name))
		{
			entry->SetEnabled(true);
		}
	}
}
void KPluginManagerBethesdaGeneric::SaveNativeOrderBG() const
{
	bool bModFileDate = ShouldChangeFileModificationDate();

	// Initialize starting time point to (current time - entries count) minutes,
	// so incrementing it by one minute gives no "overflows" into future.
	wxDateTime tFileTime = wxDateTime::Now() - wxTimeSpan(0, GetEntries().size());
	const wxTimeSpan tTimeStep(0, 1);

	// Lists
	KxStringVector tLoadOrder;
	tLoadOrder.emplace_back(V(m_OrderFileHeader));

	KxStringVector tActiveOrder;
	tActiveOrder.emplace_back(V(m_ActiveFileHeader));

	// Write order
	for (const KModListPluginEntry& tListItem: KModManager::GetListManager().GetCurrentList().GetPlugins())
	{
		if (const KPMPluginEntry* entry = tListItem.GetPluginEntry())
		{
			tLoadOrder.emplace_back(OnWriteToLoadOrder(entry));
			if (tListItem.IsEnabled())
			{
				tActiveOrder.emplace_back(OnWriteToActiveOrder(entry));
			}

			if (bModFileDate)
			{
				KxFile(entry->GetFullPath()).SetFileTime(tFileTime, KxFILETIME_MODIFICATION);
				tFileTime.Add(tTimeStep);
			}
		}
	}

	// Save files
	KxFile(m_OrderListFile.BeforeLast('\\')).CreateFolder();
	KxTextFile::WriteToFile(m_OrderListFile, tLoadOrder, wxTextFileType_Dos);

	KxFile(m_ActiveListFile.BeforeLast('\\')).CreateFolder();
	KxTextFile::WriteToFile(m_ActiveListFile, tActiveOrder, wxTextFileType_Dos);
}

KPluginManagerBethesdaGeneric::KPluginManagerBethesdaGeneric(const wxString& interfaceName, const KxXMLNode& configNode, const KPluginManagerConfig* profilePluginConfig)
	:KPluginManager(interfaceName, configNode, profilePluginConfig), m_PluginsLocation("Data")
{
	m_ActiveListFile = V(configNode.GetFirstChildElement("ActiveList").GetValue());
	m_OrderListFile = V(configNode.GetFirstChildElement("OrderList").GetValue());

	// Don't expand them here. These strings may contain date-time variables
	m_ActiveFileHeader = configNode.GetFirstChildElement("ActiveListHeader").GetValue();
	m_OrderFileHeader = configNode.GetFirstChildElement("OrderListHeader").GetValue();

	m_ChangeFileModificationDate = KAux::StringToBool(configNode.GetFirstChildElement("ChangeFileModificationDate").GetValue());
	m_SortByFileModificationDate = KAux::StringToBool(configNode.GetFirstChildElement("SortByFileModificationDate").GetValue());
}
KPluginManagerBethesdaGeneric::~KPluginManagerBethesdaGeneric()
{
}

bool KPluginManagerBethesdaGeneric::IsOK() const
{
	return true;
}
KWorkspace* KPluginManagerBethesdaGeneric::GetWorkspace() const
{
	return m_Workspace;
}

bool KPluginManagerBethesdaGeneric::Save()
{
	SaveNativeOrderBG();
	return true;
}
bool KPluginManagerBethesdaGeneric::Load()
{
	Clear();

	auto files = KModManager::GetDispatcher().FindFiles(m_PluginsLocation, KxFile::NullFilter, KxFS_FILE, false);
	KModList& tLoadOrder = KModManager::GetListManager().GetCurrentList();

	for (const KModListPluginEntry& tListEntry: tLoadOrder.GetPlugins())
	{
		// Find whether plugin with this name exist
		wxString nameL = KxString::ToLower(tListEntry.GetPluginName());
		auto it = std::find_if(files.begin(), files.end(), [&nameL](const KxFileFinderItem& item)
		{
			return KxString::ToLower(item.GetName()) == nameL;
		});

		if (!nameL.StartsWith('#') && it != files.end())
		{
			KPMPluginEntryType type = GetPluginTypeFromPath(nameL);
			if (type != KPMPE_TYPE_INVALID)
			{
				KPMPluginEntry* entry = EmplacePluginEntry(tListEntry.GetPluginName(), false, type);
				entry->SetFullPath(it->GetFullPath());
				SetParentModAndPluginFile(entry);
			}
		}
	}

	// Load files form 'Data' folder. Don't add already existing
	for (const KxFileFinderItem& item: files)
	{
		KPMPluginEntryType type = GetPluginTypeFromPath(item.GetName());
		if (type != KPMPE_TYPE_INVALID)
		{
			if (FindPluginByName(item.GetName()) == NULL)
			{
				KPMPluginEntry* entry = EmplacePluginEntry(item.GetName(), false, type);
				entry->SetFullPath(item.GetFullPath());
				SetParentModAndPluginFile(entry);
			}
		}
	}

	// Check active
	for (const KModListPluginEntry& tListEntry: tLoadOrder.GetPlugins())
	{
		KPMPluginEntry* entry = FindPluginByName(tListEntry.GetPluginName());
		if (entry)
		{
			entry->SetEnabled(tListEntry.IsEnabled());
		}
	}

	// Sort by file modification date if needed otherwise all elements already in right order
	if (ShouldSortByFileModificationDate())
	{
		SortByDate();
	}

	KModManager::GetListManager().SyncList(tLoadOrder.GetID());
	return true;
}
bool KPluginManagerBethesdaGeneric::LoadNativeOrder()
{
	LoadNativeOrderBG();
	Save();
	return true;
}

const KModEntry* KPluginManagerBethesdaGeneric::GetParentMod(const KPMPluginEntry* pluginEntry) const
{
	KModEntry* pOwningMod = NULL;
	KModManager::GetDispatcher().GetTargetPath(GetPluginRootRelativePath(pluginEntry->GetName()), &pOwningMod);
	return pOwningMod;
}
