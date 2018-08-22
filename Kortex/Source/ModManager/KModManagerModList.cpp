#include "stdafx.h"
#include "KModManagerModList.h"
#include "KModManager.h"
#include "KModEntry.h"
#include "PluginManager/KPluginManager.h"
#include "Profile/KProfile.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFileStream.h>

KModListModEntry::KModListModEntry(const wxString& signature, bool enabled)
	:m_ModEntry(KModManager::Get().FindModBySignature(signature)), m_IsEnabled(enabled)
{
}

KModListPluginEntry::KModListPluginEntry(KPluginEntry* pluginEntry, bool enabled)
	:m_PluginName(pluginEntry->GetName()), m_IsEnabled(enabled)
{
}
KModListPluginEntry::KModListPluginEntry(const wxString& name, bool enabled)
	: m_PluginName(name), m_IsEnabled(enabled)
{
}

KPluginEntry* KModListPluginEntry::GetPluginEntry() const
{
	if (KPluginManager* manager = KPluginManager::GetInstance())
	{
		return manager->FindPluginByName(m_PluginName);
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
void KModList::SetID(const wxString& id)
{
	KModManager::GetListManager().DoRenameList(*this, id);
	m_ID = id;
}

//////////////////////////////////////////////////////////////////////////
void KModManagerModList::DoChangeCurrentListID(const wxString& id)
{
	m_CurrentListID = id;
	m_Options.SetAttribute("CurrentList", m_CurrentListID);

	UpdateWriteTargetLocation();
}
void KModManagerModList::UpdateWriteTargetLocation()
{
	KModEntry* writeTargetRoot = KModManager::Get().GetModEntry_WriteTarget();
	writeTargetRoot->SetLinkedModLocation(GetWriteTargetFullPath(m_CurrentListID));
	writeTargetRoot->UpdateFileTree();

	// Create folder
	KxFile(writeTargetRoot->GetLocation(KMM_LOCATION_MOD_FILES)).CreateFolder();

	// Update variable
	KProfile* profile = KApp::Get().GetCurrentProfile();
	profile->GetVariables().SetVariable(KVAR_WRITE_TARGET_ROOT, writeTargetRoot->GetLocation(KMM_LOCATION_MOD_FILES));
}
void KModManagerModList::DoRenameList(KModList& list, const wxString& newID)
{
	// Rename write target folder
	KxFile(GetWriteTargetFullPath(list.GetID())).Rename(GetWriteTargetFullPath(newID), false);
	if (IsCurrentListID(newID))
	{
		UpdateWriteTargetLocation();
	}
}

wxString KModManagerModList::CreateListName(size_t pos) const
{
	return wxString::Format("List #%zu", pos);
}

KModManagerModList::KModManagerModList()
	:m_Options(&KModManager::Get(), "ListManager")
{
}
KModManagerModList::~KModManagerModList()
{
}

bool KModManagerModList::SetCurrentListID(const wxString& id)
{
	// Change list only if new ID is valid
	if (FindModList(id))
	{
		DoChangeCurrentListID(id);
		return true;
	}
	return false;
}

void KModManagerModList::ReloadLists()
{
	ClearLists();
	wxString currentListID = m_Options.GetAttribute("CurrentList", GetDefaultListID());

	KxFileStream stream(KModManager::Get().GetLocation(KMM_LOCATION_MODS_ORDER), KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING);
	KxXMLDocument xml(stream);
	if (stream.IsOk())
	{
		auto LoadList = [this](const KxXMLNode& listNode, const wxString& id)
		{
			KModList& list = CreateNewList(id);

			// Mods
			KxXMLNode tModsNode = listNode.GetFirstChildElement("Mods");
			for (KxXMLNode entryNode = tModsNode.GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
			{
				KModListModEntry& listEntry = list.GetMods().emplace_back(entryNode.GetAttribute("Signature"), entryNode.GetAttributeBool("Enabled", false));
				if (!listEntry.IsOK())
				{
					list.GetMods().pop_back();
				}
			}

			// Plugins
			if (KPluginManager::HasInstance())
			{
				KxXMLNode pluginsNode = listNode.GetFirstChildElement("Plugins");
				for (KxXMLNode entryNode = pluginsNode.GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
				{
					KModListPluginEntry& listEntry = list.GetPlugins().emplace_back(entryNode.GetAttribute("Name"), entryNode.GetAttributeBool("Enabled", false));
					if (!listEntry.IsOK())
					{
						list.GetPlugins().pop_back();
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
	DoChangeCurrentListID(currentListID);
}
void KModManagerModList::SaveLists()
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

			// Mods
			KxXMLNode modsNode = listNode.NewElement("Mods");
			for (const KModListModEntry& listEntry: modList.GetMods())
			{
				KxXMLNode node = modsNode.NewElement("Entry");
				node.SetAttribute("Signature", listEntry.GetMod()->GetSignature());
				node.SetAttribute("Enabled", listEntry.IsEnabled());
			}

			// Plugins
			if (KPluginManager* pluginManager = KPluginManager::GetInstance())
			{
				KxXMLNode pluginsNode = listNode.NewElement("Plugins");
				for (const KModListPluginEntry& listEntry: modList.GetPlugins())
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
bool KModManagerModList::SyncList(const wxString& id)
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

KModList& KModManagerModList::CreateNewList(const wxString& id)
{
	return m_Lists.emplace_back(id.IsEmpty() ? CreateListName(m_Lists.size() + 1) : id);
}
KModList& KModManagerModList::CreateListCopy(const KModList& list, const wxString& newID)
{
	KModList& newList = m_Lists.emplace_back(list);
	newList.SetID(newID.IsEmpty() ? CreateListName(m_Lists.size()) : newID);
	return newList;
}
KModList* KModManagerModList::RenameList(const wxString& oldID, const wxString& newID)
{
	auto it = FindModListIterator(oldID);
	if (it != m_Lists.end())
	{
		DoRenameList(*it, newID);
		return &*it;
	}
	return NULL;
}
bool KModManagerModList::RemoveList(const wxString& id)
{
	// Original can be reference to removed item
	wxString idCopy = id;

	auto it = FindModListIterator(idCopy);
	if (it != m_Lists.end())
	{
		m_Lists.erase(it);
		KxFile(GetWriteTargetFullPath(idCopy)).RemoveFolderTree(true, true);

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

wxString KModManagerModList::GetWriteTargetName(const wxString& id) const
{
	return "WriteTargetRoot-" + KModEntry::GetSignatureFromID(id);
}
wxString KModManagerModList::GetWriteTargetFullPath(const wxString& id) const
{
	return KProfile::GetCurrent()->GetRCPD({GetWriteTargetName(id)});
}
