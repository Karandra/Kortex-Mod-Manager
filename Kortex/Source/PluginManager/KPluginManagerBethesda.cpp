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
#include "ProgramManager/KProgramManager.h"
#include "ModManager/KDispatcher.h"
#include "Profile/KProfile.h"
#include "GameInstance/KInstanceManagement.h"
#include "GameInstance/Config/KPluginManagerConfig.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxProcess.h>
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxComparator.h>
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

	for (auto& entry: GetEntries())
	{
		if (entry->IsEnabled())
		{
			const KPluginReaderBethesda* bethesdaReader = NULL;
			if (entry->HasReader() && entry->GetReader()->As(bethesdaReader))
			{
				KxStringVector dependenciesList = bethesdaReader->GetRequiredPlugins();
				auto it = std::find_if(dependenciesList.begin(), dependenciesList.end(), [&pluginEntry](const wxString& depName)
				{
					return KxComparator::IsEqual(pluginEntry.GetName(), depName);
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
	const wxString ext = name.AfterLast('.');
	return KxComparator::IsEqual(ext, wxS("esp")) || KxComparator::IsEqual(ext, wxS("esm"));
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
	KFileTreeNode::CRefVector files = KModManager::GetDispatcher().FindFiles(m_PluginsLocation, KxFile::NullFilter, KxFS_FILE, false);

	// Load from 'LoadOrder.txt'
	for (const wxString& name: KxTextFile::ReadToArray(V(m_OrderListFile)))
	{
		// Find whether plugin with this name exist
		auto it = std::find_if(files.begin(), files.end(), [&name](const KFileTreeNode* node)
		{
			return KxComparator::IsEqual(node->GetName(), name);
		});

		if (!name.StartsWith('#') && it != files.end())
		{
			if (CheckExtension(name))
			{
				auto& entry = GetEntries().emplace_back(NewPluginEntry(name, false));
				entry->SetFullPath((*it)->GetFullPath());
				entry->SetParentMod(FindParentMod(*entry));
			}
		}
	}

	// Load new plugins from folder
	for (const KFileTreeNode* fileNode: files)
	{
		if (CheckExtension(KxString::ToLower(fileNode->GetName())) && !FindPluginByName(fileNode->GetName()))
		{
			auto& entry = GetEntries().emplace_back(NewPluginEntry(fileNode->GetName(), false));
			entry->SetFullPath(fileNode->GetFullPath());
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
	for (const wxString& name: KxTextFile::ReadToArray(V(m_ActiveListFile)))
	{
		if (KPluginEntry* entry = FindPluginByName(name))
		{
			entry->SetEnabled(true);
		}
	}
}
void KPluginManagerBethesda::SaveNativeOrderBG() const
{
	const bool modFileDate = ShouldChangeFileModificationDate();

	// Initialize starting time point to (current time - entries count) minutes,
	// so incrementing it by one minute gives no "overflows" into future.
	wxDateTime fileTime = wxDateTime::Now() - wxTimeSpan(0, GetEntries().size());
	const wxTimeSpan timeStep(0, 1);

	// Lists
	KxStringVector loadOrder;
	loadOrder.emplace_back(V(m_OrderFileHeader));

	KxStringVector activeOrder;
	activeOrder.emplace_back(V(m_ActiveFileHeader));

	// Write order
	for (const KProfilePlugin& listItem: KGameInstance::GetActive()->GetActiveProfile()->GetPlugins())
	{
		if (const KPluginEntry* entry = listItem.GetPlugin())
		{
			loadOrder.emplace_back(OnWriteToLoadOrder(*entry));
			if (listItem.IsEnabled())
			{
				activeOrder.emplace_back(OnWriteToActiveOrder(*entry));
			}

			if (modFileDate)
			{
				KxFile(entry->GetFullPath()).SetFileTime(fileTime, KxFILETIME_MODIFICATION);
				fileTime.Add(timeStep);
			}
		}
	}

	// Save files
	const wxString orderListFile = V(m_OrderListFile);
	KxFile(orderListFile.BeforeLast('\\')).CreateFolder();
	KxTextFile::WriteToFile(orderListFile, loadOrder, wxTextFileType_Dos);

	const wxString activeListFile = V(m_ActiveListFile);
	KxFile(activeListFile.BeforeLast('\\')).CreateFolder();
	KxTextFile::WriteToFile(activeListFile, activeOrder, wxTextFileType_Dos);
}

KPluginManagerBethesda::KPluginManagerBethesda(const wxString& interfaceName, const KxXMLNode& configNode)
	:KPluginManager(interfaceName, configNode), m_PluginsLocation("Data")
{
	m_ActiveListFile = configNode.GetFirstChildElement("ActiveList").GetValue();
	m_OrderListFile = configNode.GetFirstChildElement("OrderList").GetValue();

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
		return V("$T(PluginManager.PluginType.Master) ($T(PluginManager.PluginType.Light))");
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

void KPluginManagerBethesda::Save() const
{
	SaveNativeOrderBG();
}
void KPluginManagerBethesda::Load()
{
	Clear();

	if (KProfile* profile = KGameInstance::GetActiveProfile())
	{
		KFileTreeNode::CRefVector files = KModManager::GetDispatcher().FindFiles(m_PluginsLocation, KxFile::NullFilter, KxFS_FILE, false, true);

		for (const KProfilePlugin& listEntry: profile->GetPlugins())
		{
			// Find whether plugin with this name exist
			auto it = std::find_if(files.begin(), files.end(), [&listEntry](const KFileTreeNode* item)
			{
				return KxComparator::IsEqual(item->GetName(), listEntry.GetPluginName());
			});

			if (it != files.end())
			{
				if (CheckExtension(listEntry.GetPluginName()))
				{
					auto& entry = GetEntries().emplace_back(NewPluginEntry(listEntry.GetPluginName(), false));
					entry->SetFullPath((*it)->GetFullPath());
					entry->SetParentMod(FindParentMod(*entry));
				}
			}
		}

		// Load files form 'Data' folder. Don't add already existing
		for (const KFileTreeNode* fileNode: files)
		{
			if (CheckExtension(fileNode->GetName()))
			{
				if (FindPluginByName(fileNode->GetName()) == NULL)
				{
					auto& entry = GetEntries().emplace_back(NewPluginEntry(fileNode->GetName(), false));
					entry->SetFullPath(fileNode->GetFullPath());
					entry->SetParentMod(FindParentMod(*entry));
				}
			}
		}

		// Check active
		for (const KProfilePlugin& listEntry: profile->GetPlugins())
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
		profile->SyncWithCurrentState();
	}
}
void KPluginManagerBethesda::LoadNativeOrder()
{
	LoadNativeOrderBG();
	ReadPluginsData();

	Save();
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
	const KFileTreeNode* node = KModManager::GetDispatcher().ResolveLocation(GetPluginRootRelativePath(pluginEntry.GetName()));
	if (node)
	{
		return &node->GetMod();
	}
	return NULL;
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
