#include "stdafx.h"
#include "IGameInstance.h"
#include "DefaultGameProfile.h"
#include <Kortex/Application.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/PluginManager.hpp>
#include <KxFramework/KxFileStream.h>

namespace Kortex::GameInstance
{
	void DefaultGameProfile::OnConfigChanged(IAppOption& option)
	{
		wxLogInfo("DefaultGameProfile::OnConfigChanged -> %s", option.GetXPath());
	}
	void DefaultGameProfile::SaveConfig()
	{
		KxXMLNode rootNode = m_Config.QueryOrCreateElement("Profile");

		KxXMLNode configNode = rootNode.QueryOrCreateElement("Config");
		configNode.QueryOrCreateElement("LocalSaves").SetAttribute("Enabled", m_LocalSavesEnabled);
		configNode.QueryOrCreateElement("LocalConfig").SetAttribute("Enabled", m_LocalConfigEnabled);

		if (Kortex::IModManager::HasInstance())
		{
			KxXMLNode modsNode = rootNode.QueryOrCreateElement("Mods");
			modsNode.ClearChildren();

			for (const ProfileMod& mod: m_Mods)
			{
				KxXMLNode node = modsNode.NewElement("Entry");
				node.SetAttribute("Signature", mod.GetSignature());
				node.SetAttribute("Enabled", mod.IsActive());
			}
		}

		if (Kortex::IPluginManager::HasInstance())
		{
			KxXMLNode pluginsNode = rootNode.QueryOrCreateElement("Plugins");
			pluginsNode.ClearChildren();

			for (const ProfilePlugin& plugin: m_Plugins)
			{
				KxXMLNode node = pluginsNode.NewElement("Entry");
				node.SetAttribute("Name", plugin.GetPluginName());
				node.SetAttribute("Enabled", plugin.IsActive());
			}
		}

		KxFileStream stream(GetConfigFile(), KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Read);
		m_Config.Save(stream);
	}
	void DefaultGameProfile::LoadConfig()
	{
		KxFileStream stream(GetConfigFile(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
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
				ProfileMod& mod = m_Mods.emplace_back(node.GetAttribute("Signature"), node.GetAttributeBool("Enabled"));
				if (!mod.IsOK())
				{
					m_Mods.pop_back();
				}
			}

			m_Plugins.clear();
			KxXMLNode pluginsNode = rootNode.GetFirstChildElement("Plugins");
			for (KxXMLNode node = pluginsNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
			{
				ProfilePlugin& plugin = m_Plugins.emplace_back(node.GetAttribute("Name"), node.GetAttributeBool("Enabled"));
				if (!plugin.IsOK())
				{
					m_Plugins.pop_back();
				}
			}
		}
	}
	
	std::unique_ptr<IGameProfile> DefaultGameProfile::Clone() const
	{
		auto profile = std::make_unique<DefaultGameProfile>();
		profile->m_ID = m_ID;
		profile->LoadConfig();

		return profile;
	}
	void DefaultGameProfile::SyncWithCurrentState()
	{
		// Mods
		m_Mods.clear();
		for (auto& entry: IModManager::GetInstance()->GetMods())
		{
			m_Mods.emplace_back(*entry, entry->IsActive());
		}

		// Plugins
		m_Plugins.clear();
		if (IPluginManager* manager = IPluginManager::GetInstance())
		{
			for (auto& plugin: manager->GetPlugins())
			{
				m_Plugins.emplace_back(*plugin, plugin->IsActive());
			}
		}
	}

	void DefaultGameProfile::SetLocalSavesEnabled(bool value)
	{
		m_LocalSavesEnabled = value;
		SaveConfig();
		if (IsActive())
		{
			IGameInstance::GetActive()->GetVariables().SetVariable(Variables::KVAR_SAVES_DIR, GetSavesDir());
		}

		if (value)
		{
			CreateLocalFolder(m_ID, FolderName::Saves);
		}
	}
	void DefaultGameProfile::SetLocalConfigEnabled(bool value)
	{
		m_LocalConfigEnabled = value;
		SaveConfig();
		if (IsActive())
		{
			IGameInstance::GetActive()->GetVariables().SetVariable(Variables::KVAR_CONFIG_DIR, GetConfigDir());
		}

		if (value)
		{
			CreateLocalFolder(m_ID, FolderName::Config);
		}
	}
}
