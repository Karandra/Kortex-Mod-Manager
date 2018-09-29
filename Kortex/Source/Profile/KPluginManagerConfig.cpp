#include "stdafx.h"
#include "KPluginManagerConfig.h"
#include "KProfile.h"
#include "PluginManager/KPluginManager.h"
#include "PluginManager/LOOT API/KLootAPI.h"
#include "KApp.h"
#include "KAux.h"

KPluginManagerConfigStdContentEntry::KPluginManagerConfigStdContentEntry(KxXMLNode& node)
{
	m_ID = node.GetAttribute("ID");
	m_Name = V(node.GetAttribute("Name"));
	m_Logo = V(node.GetAttribute("Logo"));
}
KPluginManagerConfigStdContentEntry::~KPluginManagerConfigStdContentEntry()
{
}

wxString KPluginManagerConfigStdContentEntry::GetLogoFullPath() const
{
	return wxString::Format("%s\\PluginManager\\Logos\\%s\\%s", KApp::Get().GetDataFolder(), V("$(ID)"), GetLogo());
}

//////////////////////////////////////////////////////////////////////////
KPluginManagerConfigSortingToolEntry::KPluginManagerConfigSortingToolEntry(KxXMLNode& node)
{
	m_ID = node.GetAttribute("ID");
	m_Name = V(node.GetAttribute("Name"));
	m_Command = V(node.GetFirstChildElement("Command").GetValue());
}
KPluginManagerConfigSortingToolEntry::~KPluginManagerConfigSortingToolEntry()
{
}

wxString KPluginManagerConfigSortingToolEntry::GetExecutable() const
{
	if (KPluginManager* manager = KPluginManager::GetInstance())
	{
		return manager->GetSortingToolsOptions().GetAttribute(m_ID);
	}
	return wxEmptyString;
}
void KPluginManagerConfigSortingToolEntry::SetExecutable(const wxString& path) const
{
	if (KPluginManager* manager = KPluginManager::GetInstance())
	{
		manager->GetSortingToolsOptions().SetAttribute(m_ID, path);
	}
}

//////////////////////////////////////////////////////////////////////////
KPluginManagerConfigLootAPI::KPluginManagerConfigLootAPI(KProfile& profile, const KxXMLNode& node)
{
	#if _WIN64
	m_Librray.Load(KApp::Get().GetDataFolder() + "\\PluginManager\\LOOT API x64\\loot_api.dll");
	#else
	m_Librray.Load(KApp::Get().GetDataFolder() + "\\PluginManager\\LOOT API x86\\loot_api.dll");
	#endif

	m_Branch = V(node.GetFirstChildElement("Branch").GetValue());
	m_Repository = V(node.GetFirstChildElement("Repository").GetValue());
	m_FolderName = V(node.GetFirstChildElement("FolderName").GetValue());
	m_LocalGamePath = V(node.GetFirstChildElement("LocalGamePath").GetValue());

	m_LootInstance = new KLootAPI();
}
KPluginManagerConfigLootAPI::~KPluginManagerConfigLootAPI()
{
	delete m_LootInstance;
}

//////////////////////////////////////////////////////////////////////////
KPluginManagerConfig::KPluginManagerConfig(KProfile& profile, KxXMLNode& node)
	:m_InterfaceName(node.GetAttribute("InterfaceName")),
	m_PluginFileFormat(node.GetAttribute("PluginFileFormat"))
{
	// Create manager
	if (IsOK())
	{
		m_Manager = KPluginManager::QueryInterface(m_InterfaceName, node);
	}

	// Plugin limit
	m_PluginLimit = node.GetFirstChildElement("Limit").GetAttributeInt("Value", -1);

	// Load std content
	KxXMLNode stdContentNode = node.GetFirstChildElement("StandardContent");
	m_StandardContent_MainID = stdContentNode.GetAttribute("MainID");
	for (KxXMLNode entryNode = stdContentNode.GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
	{
		m_StandardContent.emplace_back(StandardContentEntry(entryNode));
	}

	// Load sorting tools
	for (KxXMLNode entryNode = node.GetFirstChildElement("SortingTools").GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
	{
		m_SortingTools.emplace_back(SortingToolEntry(entryNode));
	}

	// LootAPI
	KxXMLNode lootAPINode = node.GetFirstChildElement("LOOTAPI");
	if (lootAPINode.IsOK())
	{
		m_LootAPI = std::make_unique<LootAPI>(profile, lootAPINode);
	}
}
KPluginManagerConfig::~KPluginManagerConfig()
{
}

const KPluginManagerConfig::StandardContentEntry* KPluginManagerConfig::GetStandardContent(const wxString& id) const
{
	auto it = std::find_if(m_StandardContent.cbegin(), m_StandardContent.cend(), [&id](const KPluginManagerConfigStdContentEntry& entry)
	{
		return entry.GetID().IsSameAs(id, false);
	});
	if (it != m_StandardContent.cend())
	{
		return &(*it);
	}
	return NULL;
}
