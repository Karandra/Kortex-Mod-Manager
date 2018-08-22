#include "stdafx.h"
#include "KVariablesDatabase.h"
#include "KPluginManagerBethesda.h"
#include "KPluginManagerWorkspace.h"
#include "KPluginViewModelBethesda.h"
#include "KPluginManager.h"
#include "KPluginReader.h"
#include "KPluginReaderBethesda.h"
#include "UI/KWorkspace.h"
#include "UI/KWorkspaceController.h"
#include "ModManager/KModManager.h"
#include "ModManager/KModEntry.h"
#include "RunManager/KRunManager.h"
#include "ModManager/KModManagerDispatcher.h"
#include "Profile/KPluginManagerConfig.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxProcess.h>
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxProgressDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>

void KPluginManagerBethesda::SortByDate()
{
	std::sort(GetEntries().begin(), GetEntries().end(), [this](const auto& entry1, const auto& entry2)
	{
		wxFileName file1(entry1->GetFullPath());
		wxFileName file2(entry2->GetFullPath());

		return file1.GetModificationTime() < file2.GetModificationTime();
	});
}

KPluginEntry::RefVector KPluginManagerBethesda::CollectDependentPlugins(const KPluginEntry& pluginEntry, bool firstOnly) const
{
	KPluginEntry::RefVector dependentList;
	if (firstOnly)
	{
		dependentList.reserve(1);
	}

	const wxString nameL = KxString::ToLower(pluginEntry.GetName());
	for (auto& entry: GetEntries())
	{
		if (entry->IsEnabled())
		{
			const KPluginReaderBethesda* bethesdaReader = NULL;
			if (entry->HasReader() && entry->GetReader()->As(bethesdaReader))
			{
				KxStringVector dependenciesList = bethesdaReader->GetRequiredPlugins();
				auto it = std::find_if(dependenciesList.begin(), dependenciesList.end(), [&nameL](const wxString& sDepName)
				{
					return nameL == KxString::ToLower(sDepName);
				});
				if (it != dependenciesList.end())
				{
					dependentList.push_back(entry.get());
					if (firstOnly)
					{
						break;
					}
				}
			}
		}
	}

	return dependentList;
}
bool KPluginManagerBethesda::CheckExtension(const wxString& name) const
{
	wxString ext = name.AfterLast('.');
	return ext == "esp" || ext == "esm";
}

wxString KPluginManagerBethesda::OnWriteToLoadOrder(const KPluginEntry& entry) const
{
	return entry.GetName();
}
wxString KPluginManagerBethesda::OnWriteToActiveOrder(const KPluginEntry& entry) const
{
	return entry.GetName();
}
KWorkspace* KPluginManagerBethesda::CreateWorkspace(KMainWindow* mainWindow)
{
	return new KPluginManagerWorkspace(mainWindow);
}

void KPluginManagerBethesda::LoadNativeOrderBG()
{
	Clear();
	KModManagerDispatcher::FilesVector files = KModManager::GetDispatcher().FindFiles(m_PluginsLocation, KxFile::NullFilter, KxFS_FILE, false);
	KModList& loadOrder = KModManager::GetListManager().GetCurrentList();

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
			if (CheckExtension(nameL))
			{
				auto& entry = GetEntries().emplace_back(NewPluginEntry(name, false));
				entry->SetFullPath(it->GetFullPath());
				entry->SetParentMod(FindParentMod(*entry));
			}
		}
	}

	// Load new plugins from folder
	for (const KxFileFinderItem& item: files)
	{
		if (CheckExtension(KxString::ToLower(item.GetName())) && !FindPluginByName(item.GetName()))
		{
			auto& entry = GetEntries().emplace_back(NewPluginEntry(item.GetName(), false));
			entry->SetFullPath(item.GetFullPath());
			entry->SetParentMod(FindParentMod(*entry));
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
void KPluginManagerBethesda::LoadNativeActiveBG()
{
	// Load names from 'Plugins.txt' it they are not already added.
	// Activate all new added and existing items with same name.
	for (const wxString& name: KxTextFile::ReadToArray(m_ActiveListFile))
	{
		if (KPluginEntry* entry = FindPluginByName(name))
		{
			entry->SetEnabled(true);
		}
	}
}
void KPluginManagerBethesda::SaveNativeOrderBG() const
{
	bool modFileDate = ShouldChangeFileModificationDate();

	// Initialize starting time point to (current time - entries count) minutes,
	// so incrementing it by one minute gives no "overflows" into future.
	wxDateTime tFileTime = wxDateTime::Now() - wxTimeSpan(0, GetEntries().size());
	const wxTimeSpan tTimeStep(0, 1);

	// Lists
	KxStringVector loadOrder;
	loadOrder.emplace_back(V(m_OrderFileHeader));

	KxStringVector activeOrder;
	activeOrder.emplace_back(V(m_ActiveFileHeader));

	// Write order
	for (const KModListPluginEntry& listItem: KModManager::GetListManager().GetCurrentList().GetPlugins())
	{
		if (const KPluginEntry* entry = listItem.GetPluginEntry())
		{
			loadOrder.emplace_back(OnWriteToLoadOrder(*entry));
			if (listItem.IsEnabled())
			{
				activeOrder.emplace_back(OnWriteToActiveOrder(*entry));
			}

			if (modFileDate)
			{
				KxFile(entry->GetFullPath()).SetFileTime(tFileTime, KxFILETIME_MODIFICATION);
				tFileTime.Add(tTimeStep);
			}
		}
	}

	// Save files
	KxFile(m_OrderListFile.BeforeLast('\\')).CreateFolder();
	KxTextFile::WriteToFile(m_OrderListFile, loadOrder, wxTextFileType_Dos);

	KxFile(m_ActiveListFile.BeforeLast('\\')).CreateFolder();
	KxTextFile::WriteToFile(m_ActiveListFile, activeOrder, wxTextFileType_Dos);
}

KPluginManagerBethesda::KPluginManagerBethesda(const wxString& interfaceName, const KxXMLNode& configNode)
	:KPluginManager(interfaceName, configNode), m_PluginsLocation("Data")
{
	m_ActiveListFile = V(configNode.GetFirstChildElement("ActiveList").GetValue());
	m_OrderListFile = V(configNode.GetFirstChildElement("OrderList").GetValue());

	// Don't expand them here. These strings may contain date-time variables
	m_ActiveFileHeader = configNode.GetFirstChildElement("ActiveListHeader").GetValue();
	m_OrderFileHeader = configNode.GetFirstChildElement("OrderListHeader").GetValue();

	m_ShouldChangeFileModificationDate = configNode.GetFirstChildElement("ChangeFileModificationDate").GetValueBool();
	m_ShouldSortByFileModificationDate = configNode.GetFirstChildElement("SortByFileModificationDate").GetValueBool();

	m_ViewModel = std::make_unique<KPluginViewModelBethesda>();
}
KPluginManagerBethesda::~KPluginManagerBethesda()
{
}

KWorkspace* KPluginManagerBethesda::GetWorkspace() const
{
	return KPluginManagerWorkspace::GetInstance();
}
KPluginViewModel* KPluginManagerBethesda::GetViewModel() const
{
	return m_ViewModel.get();
}

KPluginEntryBethesda* KPluginManagerBethesda::NewPluginEntry(const wxString& name, bool isActive) const
{
	return new KPluginEntryBethesda(name, isActive);
}
wxString KPluginManagerBethesda::GetPluginTypeName(const KPluginEntry& pluginEntry) const
{
	const KPluginEntryBethesda* bethesdaPlugin = NULL;
	if (pluginEntry.As(bethesdaPlugin))
	{
		return GetPluginTypeName(bethesdaPlugin->IsMaster(), bethesdaPlugin->IsLight());
	}
	return GetPluginTypeName(false, false);
}
wxString KPluginManagerBethesda::GetPluginTypeName(bool isMaster, bool isLight) const
{
	if (isMaster && isLight)
	{
		return wxString::Format("%s (%s)", T("PluginManager.PluginType.Master"), T("PluginManager.PluginType.Light"));
	}
	if (isMaster)
	{
		return T("PluginManager.PluginType.Master");
	}
	if (isLight)
	{
		return T("PluginManager.PluginType.Light");
	}
	return T("PluginManager.PluginType.Normal");
}

bool KPluginManagerBethesda::Save()
{
	SaveNativeOrderBG();
	return true;
}
bool KPluginManagerBethesda::Load()
{
	Clear();

	auto files = KModManager::GetDispatcher().FindFiles(m_PluginsLocation, KxFile::NullFilter, KxFS_FILE, false);
	KModList& loadOrder = KModManager::GetListManager().GetCurrentList();

	for (const KModListPluginEntry& listEntry: loadOrder.GetPlugins())
	{
		// Find whether plugin with this name exist
		wxString nameL = KxString::ToLower(listEntry.GetPluginName());
		auto it = std::find_if(files.begin(), files.end(), [&nameL](const KxFileFinderItem& item)
		{
			return KxString::ToLower(item.GetName()) == nameL;
		});

		if (!nameL.StartsWith('#') && it != files.end())
		{
			if (CheckExtension(nameL))
			{
				auto& entry = GetEntries().emplace_back(NewPluginEntry(listEntry.GetPluginName(), false));
				entry->SetFullPath(it->GetFullPath());
				entry->SetParentMod(FindParentMod(*entry));
			}
		}
	}

	// Load files form 'Data' folder. Don't add already existing
	for (const KxFileFinderItem& item: files)
	{
		if (CheckExtension(KxString::ToLower(item.GetName())))
		{
			if (FindPluginByName(item.GetName()) == NULL)
			{
				auto& entry = GetEntries().emplace_back(NewPluginEntry(item.GetName(), false));
				entry->SetFullPath(item.GetFullPath());
				entry->SetParentMod(FindParentMod(*entry));
			}
		}
	}

	// Check active
	for (const KModListPluginEntry& listEntry: loadOrder.GetPlugins())
	{
		KPluginEntry* entry = FindPluginByName(listEntry.GetPluginName());
		if (entry)
		{
			entry->SetEnabled(listEntry.IsEnabled());
		}
	}

	// Sort by file modification date if needed otherwise all elements already in correct order
	if (ShouldSortByFileModificationDate())
	{
		SortByDate();
	}

	ReadPluginsData();
	KModManager::GetListManager().SyncList(loadOrder.GetID());
	return true;
}
bool KPluginManagerBethesda::LoadNativeOrder()
{
	LoadNativeOrderBG();
	ReadPluginsData();

	Save();
	return true;
}

bool KPluginManagerBethesda::HasDependentPlugins(const KPluginEntry& pluginEntry) const
{
	return !CollectDependentPlugins(pluginEntry, true).empty();
}
KPluginEntry::RefVector KPluginManagerBethesda::GetDependentPlugins(const KPluginEntry& pluginEntry) const
{
	return CollectDependentPlugins(pluginEntry, false);
}
const KModEntry* KPluginManagerBethesda::FindParentMod(const KPluginEntry& pluginEntry) const
{
	KModEntry* owningMod = NULL;
	KModManager::GetDispatcher().GetTargetPath(GetPluginRootRelativePath(pluginEntry.GetName()), &owningMod);
	return owningMod;
}

intptr_t KPluginManagerBethesda::GetPluginPriority(const KPluginEntry& modEntry) const
{
	intptr_t priority = 0;
	intptr_t inactive = 0;

	for (const auto& entry: GetEntries())
	{
		if (entry.get() == &modEntry)
		{
			return priority - inactive;
		}

		priority++;
		if (!entry->IsEnabled())
		{
			inactive++;
		}
	}
	return -1;
}
