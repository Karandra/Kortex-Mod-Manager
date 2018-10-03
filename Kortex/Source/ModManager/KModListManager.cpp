#include "stdafx.h"
#include "KModListManager.h"
#include "KModManager.h"
#include "KModEntry.h"
#include "PluginManager/KPluginManager.h"
#include "Profile/KProfile.h"
#include "Profile/KSaveManagerConfig.h"
#include "Events/KModListEventInternal.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFileStream.h>

KModListMod::KModListMod(const wxString& signature, bool enabled)
	:m_ModEntry(KModManager::Get().FindModBySignature(signature)), m_IsEnabled(enabled)
{
	m_IsEnabled = m_IsEnabled && (m_ModEntry ? m_ModEntry->IsInstalled() : false);
}

KModListPlugin::KModListPlugin(KPluginEntry* pluginEntry, bool enabled)
	:m_PluginName(pluginEntry->GetName()), m_IsEnabled(enabled)
{
}
KModListPlugin::KModListPlugin(const wxString& name, bool enabled)
	: m_PluginName(name), m_IsEnabled(enabled)
{
}

KPluginEntry* KModListPlugin::GetPluginEntry() const
{
	if (KPluginManager* manager = KPluginManager::GetInstance())
	{
		return manager->FindPluginByName(m_PluginName);
	}
	return NULL;
}

wxString KModList::CreateSignature(const wxString& listID)
{
	return KModEntry::GetSignatureFromID(listID);
}

wxString KModList::GetGlobalFolderPath(const wxString& folderName)
{
	return KProfile::GetCurrent()->GetRCPD({folderName});
}
wxString KModList::GetLocalRootPath(const wxString& listID)
{
	return KProfile::GetCurrent()->GetRCPD({CreateSignature(listID)});
}
wxString KModList::GetLocalFolderPath(const wxString& listID, const wxString& folderName)
{
	return KProfile::GetCurrent()->GetRCPD({CreateSignature(listID), folderName});
}

bool KModList::RemoveLocalRoot(const wxString& listID)
{
	// Remove to recycle bin
	return KxFile(GetLocalRootPath(listID)).RemoveFolderTree(true, true);
}
bool KModList::CreateLocalRoot(const wxString& listID)
{
	return KxFile(GetLocalRootPath(listID)).CreateFolder();
}
bool KModList::CreateLocalFolder(const wxString& listID, const wxString& folderName)
{
	return KxFile(GetLocalFolderPath(listID, folderName)).CreateFolder();
}
bool KModList::RemoveLocalFolder(const wxString& listID, const wxString& folderName)
{
	// Remove to recycle bin
	return KxFile(GetLocalFolderPath(listID, folderName)).RemoveFolderTree(true, true);
}
bool KModList::RenameLocalRoot(const wxString& oldID, const wxString& newID, wxString* newPathOut)
{
	wxString newPath = GetLocalRootPath(newID);
	if (newPathOut)
	{
		*newPathOut = newPath;
	}

	// Rename or create required folder
	return KxFile(GetLocalRootPath(oldID)).Rename(newPath, false) || KxFile(newPath).CreateFolder();
}

void KModList::OnRemove()
{
	RemoveLocalRoot();
}
bool KModList::IsCurrentList() const
{
	return m_ID == KModListManager::GetInstance()->GetCurrentListID();
}

//////////////////////////////////////////////////////////////////////////
bool KModList::SetID(const wxString& id)
{
	if (RenameLocalRoot(m_ID, id))
	{
		m_ID = id;

		// Current list updating
		if (IsCurrentList())
		{
			KModListManager::GetInstance()->DoChangeCurrentListID(m_ID);
		}
		
		// Save
		KModListManager::GetInstance()->SaveLists();
		KModListEvent(KEVT_MODLIST_INT_SELECTED, *this).Send();
		return true;
	}
	return false;
}
void KModList::SetLocalSavesEnabled(bool value)
{
	m_LocalSavesEnabled = value;
	KModListManager::GetInstance()->SaveLists();

	if (value)
	{
		CreateLocalFolder(m_ID, LocalFolderNames::Saves);
	}
	KModListEvent(KEVT_MODLIST_INT_SELECTED, *this).Send();
}
void KModList::SetLocalConfigEnabled(bool value)
{
	m_LocalConfigEnabled = value;
	KModListManager::GetInstance()->SaveLists();

	if (value)
	{
		CreateLocalFolder(m_ID, LocalFolderNames::Config);
	}
	KModListEvent(KEVT_MODLIST_INT_SELECTED, *this).Send();
}

//////////////////////////////////////////////////////////////////////////
void KModListManager::DoChangeCurrentListID(const wxString& id)
{
	m_Options.SetAttribute("CurrentList", id);
	m_CurrentListID = id;
}
wxString KModListManager::CreateListName(size_t pos) const
{
	return wxString::Format("List #%zu", pos);
}

void KModListManager::SetupGlobalFolders()
{
	auto SetVariableAndCreateFolder = [](const wxString& varName, const wxString& folderName)
	{
		wxString path = KModList::GetGlobalFolderPath(folderName);

		KxFile(path).CreateFolder();
		KProfile::GetCurrent()->GetVariables().SetVariable(varName, path);
	};
	SetVariableAndCreateFolder(KVAR_SAVES_ROOT_GLOBAL, GlobalFolderNames::Saves);
	SetVariableAndCreateFolder(KVAR_CONFIG_ROOT_GLOBAL, GlobalFolderNames::Config);
}
void KModListManager::OnModListSelected(KModListEvent& event)
{
	const KModList* modList = event.GetModList();
	if (modList && modList->IsCurrentList())
	{
		KIVariablesTable& variables = KProfile::GetCurrent()->GetVariables();

		// Generic variables
		variables.SetVariable(KVAR_CURRENT_MOD_LIST_ID, modList->GetID());
		variables.SetVariable(KVAR_CURRENT_MOD_LIST_SIGNATURE, modList->GetSignature());
		variables.SetVariable(KVAR_CURRENT_MOD_LIST_ROOT, modList->GetLocalRootPath());

		// Overwrites (local only)
		wxString overwitesPath = modList->GetLocalFolderPath(KModList::LocalFolderNames::Overwrites);
		variables.SetVariable(KVAR_OVERWRITES_ROOT, overwitesPath);

		KModEntry& writeTarget = *KModManager::Get().GetModEntry_WriteTarget();
		writeTarget.SetLinkedModLocation(overwitesPath);
		KxFile(overwitesPath).CreateFolder();

		// Local folders
		auto SetVariable = [&variables, modList](bool isLocal, const wxString& varName, const wxString& localName, const wxString& globalName)
		{
			wxString path;
			if (isLocal)
			{
				path = modList->GetLocalFolderPath(localName);
			}
			else
			{
				path = KModList::GetGlobalFolderPath(globalName);
			}

			KxFile(path).CreateFolder();
			variables.SetVariable(varName, path);
		};
		SetVariable(modList->IsLocalSavesEnabled(), KVAR_SAVES_ROOT_LOCAL, LocalFolderNames::Saves, GlobalFolderNames::Saves);
		SetVariable(modList->IsLocalConfigEnabled(), KVAR_CONFIG_ROOT_LOCAL, LocalFolderNames::Config, GlobalFolderNames::Config);

		// Update mods
		KModEvent::RefVector changedMods(1, &writeTarget);
		for (KModEntry& mod: KModManager::Get().GetModEntry_Mandatory())
		{
			changedMods.push_back(&mod);
		}
		KModEvent(KEVT_MOD_FILES_CHANGED, changedMods).Send();
	}
}
void KModListManager::OnInit()
{
	SetupGlobalFolders();
	LoadLists();

	KModListEvent(KEVT_MODLIST_INT_SELECTED, GetCurrentList()).Send();
}

KModListManager::KModListManager()
	:m_Options(&KModManager::Get(), "ListManager")
{
	KEvent::Bind(KEVT_MODLIST_INT_SELECTED, &KModListManager::OnModListSelected, this);
}
KModListManager::~KModListManager()
{
}

const KModList& KModListManager::GetCurrentList() const
{
	return const_cast<KModListManager*>(this)->GetCurrentList();
}
KModList& KModListManager::GetCurrentList()
{
	KModList* modList = FindModList(m_CurrentListID);

	// Default list must always exist
	return modList ? *modList : *FindModList(GetDefaultListID());
}

bool KModListManager::SetCurrentListID(const wxString& id)
{
	// Change list only if new ID is valid
	if (FindModList(id))
	{
		DoChangeCurrentListID(id);
		GetCurrentList().SetID(id);
		return true;
	}
	return false;
}

void KModListManager::LoadLists()
{
	m_Lists.clear();
	wxString currentListID = m_Options.GetAttribute("CurrentList", GetDefaultListID());

	KxFileStream stream(KModManager::Get().GetLocation(KMM_LOCATION_MODS_ORDER), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING);
	KxXMLDocument xml(stream);
	if (stream.IsOk())
	{
		auto LoadList = [this](const KxXMLNode& listNode, const wxString& id)
		{
			KModList& modList = CreateNewList(id);

			// Config
			KxXMLNode configNode = listNode.GetFirstChildElement("Config");
			modList.SetLocalSavesEnabled(configNode.GetFirstChildElement("LocalSaves").GetAttributeBool("Enabled", false));
			modList.SetLocalConfigEnabled(configNode.GetFirstChildElement("LocalConfig").GetAttributeBool("Enabled", false));

			// Mods
			KxXMLNode modsNode = listNode.GetFirstChildElement("Mods");
			for (KxXMLNode entryNode = modsNode.GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
			{
				KModListMod& listEntry = modList.GetMods().emplace_back(entryNode.GetAttribute("Signature"), entryNode.GetAttributeBool("Enabled", false));
				if (!listEntry.IsOK())
				{
					modList.GetMods().pop_back();
				}
			}

			// Plugins
			if (KPluginManager::HasInstance())
			{
				KxXMLNode pluginsNode = listNode.GetFirstChildElement("Plugins");
				for (KxXMLNode entryNode = pluginsNode.GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
				{
					KModListPlugin& listEntry = modList.GetPlugins().emplace_back(entryNode.GetAttribute("Name"), entryNode.GetAttributeBool("Enabled", false));
					if (!listEntry.IsOK())
					{
						modList.GetPlugins().pop_back();
					}
				}
			}
		};

		KxXMLNode orderNode = xml.GetFirstChildElement("Order");
		if (orderNode.GetFirstChildElement("List").IsOK())
		{
			for (KxXMLNode node = orderNode.GetFirstChildElement("List"); node.IsOK(); node = node.GetNextSiblingElement("List"))
			{
				wxString id = node.GetAttribute("ID");
				if (id.IsEmpty())
				{
					id = CreateListName(node.GetIndexWithinParent());
				}
				LoadList(node, id);
			}
		}
		else
		{
			// For versions before 1.3
			LoadList(orderNode, GetDefaultListID());
		}
	}

	// If no mod lists loaded, create an empty mod list.
	if (m_Lists.empty())
	{
		currentListID = GetDefaultListID();
		CreateNewList(currentListID);
	}

	// Check current list
	if (!HasList(currentListID))
	{
		currentListID = m_Lists.front().GetID();
	}

	// Change now
	SetCurrentListID(currentListID);
}
void KModListManager::SaveLists()
{
	KxFileStream stream(KModManager::Get().GetLocation(KMM_LOCATION_MODS_ORDER), KxFS_ACCESS_WRITE, KxFS_DISP_CREATE_ALWAYS);
	if (stream.IsOk())
	{
		KxXMLDocument xml(wxEmptyString);
		KxXMLNode orderNode = xml.NewElement("Order");

		for (const KModList& modList: GetLists())
		{
			KxXMLNode listNode = orderNode.NewElement("List");
			listNode.SetAttribute("ID", modList.GetID());

			// Config
			KxXMLNode configNode = listNode.NewElement("Config");
			configNode.NewElement("LocalSaves").SetAttribute("Enabled", modList.IsLocalSavesEnabled());
			configNode.NewElement("LocalConfig").SetAttribute("Enabled", modList.IsLocalConfigEnabled());

			// Mods
			KxXMLNode modsNode = listNode.NewElement("Mods");
			for (const KModListMod& listEntry: modList.GetMods())
			{
				KxXMLNode node = modsNode.NewElement("Entry");
				node.SetAttribute("Signature", listEntry.GetMod()->GetSignature());
				node.SetAttribute("Enabled", listEntry.IsEnabled());
			}

			// Plugins
			if (KPluginManager* pluginManager = KPluginManager::GetInstance())
			{
				KxXMLNode pluginsNode = listNode.NewElement("Plugins");
				for (const KModListPlugin& listEntry: modList.GetPlugins())
				{
					KxXMLNode node = pluginsNode.NewElement("Entry");
					node.SetAttribute("Name", listEntry.GetPluginName());
					node.SetAttribute("Enabled", listEntry.IsEnabled());
				}
			}
		}
		xml.Save(stream);
	}
}
bool KModListManager::SyncList(const wxString& id)
{
	KModList* list = FindModList(id);
	if (list)
	{
		// Mods
		list->GetMods().clear();
		for (KModEntry* entry: KModManager::Get().GetEntries())
		{
			list->GetMods().emplace_back(entry, entry->IsEnabled());
		}

		// Plugins
		if (KPluginManager* manager = KPluginManager::GetInstance())
		{
			list->GetPlugins().clear();
			for (auto& entry: manager->GetEntries())
			{
				list->GetPlugins().emplace_back(entry.get(), entry->IsEnabled());
			}
		}
		return true;
	}
	return false;
}

KModList& KModListManager::CreateNewList(const wxString& id)
{
	return m_Lists.emplace_back(id.IsEmpty() ? CreateListName(m_Lists.size() + 1) : id);
}
KModList& KModListManager::CreateListCopy(const KModList& list, const wxString& newID)
{
	KModList& newList = m_Lists.emplace_back(list);
	newList.SetID(newID.IsEmpty() ? CreateListName(m_Lists.size()) : newID);
	return newList;
}
KModList* KModListManager::RenameList(const wxString& oldID, const wxString& newID)
{
	KModList* list = FindModList(oldID);
	if (list)
	{
		list->SetID(newID);
		return list;
	}
	return NULL;
}
bool KModListManager::RemoveList(const wxString& id)
{
	// Original can be reference to removed item
	wxString idCopy = id;

	auto it = FindModListIterator(m_Lists, idCopy);
	if (it != m_Lists.end())
	{
		it->OnRemove();
		m_Lists.erase(it);

		// If no lists left, create default list
		if (m_Lists.empty())
		{
			CreateNewList(GetDefaultListID());
			SetCurrentListID(GetDefaultListID());
		}
		else if (IsCurrentListID(idCopy))
		{
			// If current list was deleted, set current to the first list.
			SetCurrentListID(m_Lists.front().GetID());
		}
		return true;
	}
	return false;
}
