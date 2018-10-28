#include "stdafx.h"
#include "KProfile.h"
#include "GameInstance/KInstanceManagement.h"
#include "ModManager/KModManager.h"
#include "PluginManager/KPluginManager.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFileStream.h>

namespace Util
{
	wxString ProcessID(const wxString& id)
	{
		return KAux::MakeSafeFileName(id);
	}

	wxString GetGlobalRelativePath(const wxString& folderName)
	{
		return KGameInstance::GetActive()->GetInstanceRelativePath(wxS("GlobalProfile")) + wxS('\\') + folderName;
	}
	wxString GetLocalPath(const wxString& id)
	{
		return KGameInstance::GetActive()->GetProfilesDir() + wxS('\\') + id;
	}
	wxString GetLocalRelativePath(const wxString& id, const wxString& name)
	{
		return GetLocalPath(id) + wxS('\\') + name;
	}

	bool CreateLocalFolder(const wxString& id, const wxString& name)
	{
		return KxFile(GetLocalRelativePath(id, name)).CreateFolder();
	}
}

//////////////////////////////////////////////////////////////////////////
KProfileMod::KProfileMod(const wxString& signature, bool enabled)
	:m_Signature(signature), m_IsEnabled(enabled)
{
}
KProfileMod::KProfileMod(const KModEntry& modEntry, bool enabled)
	:m_Signature(modEntry.GetSignature()), m_IsEnabled(enabled)
{
}

KModEntry* KProfileMod::GetMod() const
{
	if (KModManager* manager = KModManager::GetInstance())
	{
		return manager->FindModBySignature(m_Signature);
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
KProfilePlugin::KProfilePlugin(const KPluginEntry& pluginEntry, bool enabled)
	:m_PluginName(pluginEntry.GetName()), m_IsEnabled(enabled)
{
}
KProfilePlugin::KProfilePlugin(const wxString& name, bool enabled)
	:m_PluginName(name), m_IsEnabled(enabled)
{
}

KPluginEntry* KProfilePlugin::GetPlugin() const
{
	if (KPluginManager* manager = KPluginManager::GetInstance())
	{
		return manager->FindPluginByName(m_PluginName);
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
void KProfile::SetGlobalPaths(KIVariablesTable& variables)
{
	variables.SetVariable(KVAR_GLOBAL_SAVES_DIR, Util::GetGlobalRelativePath(FolderNames::Saves));
	variables.SetVariable(KVAR_GLOBAL_CONFIG_DIR, Util::GetGlobalRelativePath(FolderNames::Config));
}
wxString KProfile::ProcessID(const wxString& id)
{
	return Util::ProcessID(id);
}

wxString KProfile::GetOrderFile() const
{
	return Util::GetLocalRelativePath(m_ID, wxS("Profile.xml"));
}
void KProfile::SetID(const wxString& id)
{
	m_ID = id;
}

KProfile::KProfile(const wxString& id)
	:m_ID(Util::ProcessID(id))
{
}

wxString KProfile::GetProfileDir() const
{
	return Util::GetLocalPath(m_ID);
}
wxString KProfile::GetProfileRelativePath(const wxString& name) const
{
	return Util::GetLocalRelativePath(m_ID, name);
}
wxString KProfile::GetSavesDir() const
{
	if (m_LocalSavesEnabled)
	{
		return GetProfileRelativePath(FolderNames::Saves);
	}
	else
	{
		return Util::GetGlobalRelativePath(FolderNames::Saves);
	}
}
wxString KProfile::GetConfigDir() const
{
	if (m_LocalConfigEnabled)
	{
		return GetProfileRelativePath(FolderNames::Config);
	}
	else
	{
		return Util::GetGlobalRelativePath(FolderNames::Config);
	}
}
wxString KProfile::GetOverwritesDir() const
{
	return GetProfileRelativePath(FolderNames::Overwrites);
}

void KProfile::Load()
{
	KxFileStream stream(GetOrderFile(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
	KxXMLDocument xml(stream);
	if (xml.IsOK())
	{
		KxXMLNode rootNode = xml.GetFirstChildElement("Profile");

		KxXMLNode configNode = rootNode.GetFirstChildElement("Config");
		m_LocalSavesEnabled = configNode.GetFirstChildElement("LocalSaves").GetAttributeBool("Enabled");
		m_LocalConfigEnabled = configNode.GetFirstChildElement("LocalConfig").GetAttributeBool("Enabled");

		m_Mods.clear();
		KxXMLNode modsNode = rootNode.GetFirstChildElement("Mods");
		for (KxXMLNode node = modsNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{
			KProfileMod& mod = m_Mods.emplace_back(node.GetAttribute("Signature"), node.GetAttributeBool("Enabled"));
			if (!mod.IsOK())
			{
				m_Mods.pop_back();
			}
		}

		m_Plugins.clear();
		KxXMLNode pluginsNode = rootNode.GetFirstChildElement("Plugins");
		for (KxXMLNode node = pluginsNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{
			KProfilePlugin& plugin = m_Plugins.emplace_back(node.GetAttribute("Name"), node.GetAttributeBool("Enabled"));
			if (!plugin.IsOK())
			{
				m_Plugins.pop_back();
			}
		}
	}
}
void KProfile::Save()
{
	KxXMLDocument xml;
	KxXMLNode rootNode = xml.NewElement("Profile");

	KxXMLNode configNode = rootNode.NewElement("Config");
	configNode.NewElement("LocalSaves").SetAttribute("Enabled", m_LocalSavesEnabled);
	configNode.NewElement("LocalConfig").SetAttribute("Enabled", m_LocalConfigEnabled);

	if (KModManager::HasInstance())
	{
		KxXMLNode modsNode = rootNode.NewElement("Mods");
		for (const KProfileMod& mod: m_Mods)
		{
			KxXMLNode node = modsNode.NewElement("Entry");
			node.SetAttribute("Signature", mod.GetSignature());
			node.SetAttribute("Enabled", mod.IsEnabled());
		}
	}

	if (KPluginManager::HasInstance())
	{
		KxXMLNode pluginsNode = rootNode.NewElement("Plugins");
		for (const KProfilePlugin& plugin: m_Plugins)
		{
			KxXMLNode node = pluginsNode.NewElement("Entry");
			node.SetAttribute("Name", plugin.GetPluginName());
			node.SetAttribute("Enabled", plugin.IsEnabled());
		}
	}

	KxFileStream stream(GetOrderFile(), KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Read);
	xml.Save(stream);
}
void KProfile::SyncWithCurrentState()
{
	// Mods
	m_Mods.clear();
	for (auto& entry: KModManager::GetInstance()->GetEntries())
	{
		m_Mods.emplace_back(*entry, entry->IsEnabled());
	}

	// Plugins
	m_Plugins.clear();
	if (KPluginManager* manager = KPluginManager::GetInstance())
	{
		for (auto& entry: manager->GetEntries())
		{
			m_Plugins.emplace_back(*entry, entry->IsEnabled());
		}
	}
}

bool KProfile::IsActive() const
{
	if (KGameInstance* instance = KGameInstance::GetActive())
	{
		return instance->GetActiveProfileID() == m_ID;
	}
	return false;
}

void KProfile::SetLocalSavesEnabled(bool value)
{
	m_LocalSavesEnabled = value;
	Save();
	if (IsActive())
	{
		KGameInstance::GetActive()->GetVariables().SetVariable(KVAR_SAVES_DIR, GetSavesDir());
	}

	if (value)
	{
		Util::CreateLocalFolder(m_ID, FolderNames::Saves);
	}
}
void KProfile::SetLocalConfigEnabled(bool value)
{
	m_LocalConfigEnabled = value;
	Save();
	if (IsActive())
	{
		KGameInstance::GetActive()->GetVariables().SetVariable(KVAR_CONFIG_DIR, GetConfigDir());
	}

	if (value)
	{
		Util::CreateLocalFolder(m_ID, FolderNames::Config);
	}
}
