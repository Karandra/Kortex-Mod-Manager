#include "stdafx.h"
#include "KPluginManagerConfig.h"
#include "KProfile.h"
#include "PluginManager/KPluginManager.h"
#include "PluginManager/LOOT API/KLootAPI.h"
#include "KComparator.h"
#include "KApp.h"
#include "KAux.h"

KPluginManagerConfigStdContentEntry::KPluginManagerConfigStdContentEntry(KxXMLNode& node)
{
	m_ID = node.GetAttribute("ID");
	m_Name = node.GetAttribute("Name");
	m_Logo = node.GetAttribute("Logo");
}
KPluginManagerConfigStdContentEntry::~KPluginManagerConfigStdContentEntry()
{
}

wxString KPluginManagerConfigStdContentEntry::GetID() const
{
	return m_ID;
}
wxString KPluginManagerConfigStdContentEntry::GetName() const
{
	return V(m_Name);
}
wxString KPluginManagerConfigStdContentEntry::GetLogo() const
{
	return V(m_Logo);
}

wxString KPluginManagerConfigStdContentEntry::GetLogoFullPath() const
{
	return V(wxString::Format("%s\\PluginManager\\Logos\\%s\\%s", KApp::Get().GetDataFolder(), "$(ID)", GetLogo()));
}

//////////////////////////////////////////////////////////////////////////
KPluginManagerConfigSortingToolEntry::KPluginManagerConfigSortingToolEntry(KxXMLNode& node)
{
	m_ID = node.GetAttribute("ID");
	m_Name = node.GetAttribute("Name");
	m_Command = node.GetFirstChildElement("Command").GetValue();
}
KPluginManagerConfigSortingToolEntry::~KPluginManagerConfigSortingToolEntry()
{
}

wxString KPluginManagerConfigSortingToolEntry::GetID() const
{
	return m_ID;
}
wxString KPluginManagerConfigSortingToolEntry::GetName() const
{
	return V(m_Name);
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

wxString KPluginManagerConfigSortingToolEntry::GetArguments() const
{
	return V(m_Command);
}

//////////////////////////////////////////////////////////////////////////
KPluginManagerConfigLootAPI::KPluginManagerConfigLootAPI(KProfile& profile, const KxXMLNode& node)
{
	#if _WIN64
	m_Librray.Load(KApp::Get().GetDataFolder() + "\\PluginManager\\LOOT API x64\\loot_api.dll");
	#else
	m_Librray.Load(KApp::Get().GetDataFolder() + "\\PluginManager\\LOOT API x86\\loot_api.dll");
	#endif

	m_Branch = node.GetFirstChildElement("Branch").GetValue();
	m_Repository = node.GetFirstChildElement("Repository").GetValue();
	m_FolderName = node.GetFirstChildElement("FolderName").GetValue();
	m_LocalGamePath = node.GetFirstChildElement("LocalGamePath").GetValue();

	m_LootInstance = new KLootAPI();
}
KPluginManagerConfigLootAPI::~KPluginManagerConfigLootAPI()
{
	delete m_LootInstance;
}

wxString KPluginManagerConfigLootAPI::GetBranch() const
{
	return V(m_Branch);
}
wxString KPluginManagerConfigLootAPI::GetRepository() const
{
	return V(m_Repository);
}
wxString KPluginManagerConfigLootAPI::GetFolderName() const
{
	return V(m_FolderName);
}
wxString KPluginManagerConfigLootAPI::GetLocalGamePath() const
{
	return V(m_LocalGamePath);
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

bool KPluginManagerConfig::HasMainStdContentID() const
{
	return !m_StandardContent_MainID.IsEmpty();
}
wxString KPluginManagerConfig::GetMainStdContentID() const
{
	return V(m_StandardContent_MainID);
}

const KPluginManagerConfig::StandardContentEntry* KPluginManagerConfig::GetStandardContent(const wxString& id) const
{
	auto it = std::find_if(m_StandardContent.begin(), m_StandardContent.end(), [&id](const KPluginManagerConfigStdContentEntry& entry)
	{
		return KComparator::KEqual(entry.GetID(), id);
	});
	if (it != m_StandardContent.cend())
	{
		return &*it;
	}
	return NULL;
}
