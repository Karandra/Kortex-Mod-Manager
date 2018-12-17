#include "stdafx.h"
#include "IGameInstance.h"
#include "DefaultGameProfile.h"
#include <Kortex/Application.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/PluginManager.hpp>
#include <KxFramework/KxFileStream.h>

namespace Kortex::GameInstance
{
	void DefaultGameProfile::Load()
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
	void DefaultGameProfile::Save()
	{
		KxXMLDocument xml;
		KxXMLNode rootNode = xml.NewElement("Profile");

		KxXMLNode configNode = rootNode.NewElement("Config");
		configNode.NewElement("LocalSaves").SetAttribute("Enabled", m_LocalSavesEnabled);
		configNode.NewElement("LocalConfig").SetAttribute("Enabled", m_LocalConfigEnabled);

		if (Kortex::IModManager::HasInstance())
		{
			KxXMLNode modsNode = rootNode.NewElement("Mods");
			for (const ProfileMod& mod: m_Mods)
			{
				KxXMLNode node = modsNode.NewElement("Entry");
				node.SetAttribute("Signature", mod.GetSignature());
				node.SetAttribute("Enabled", mod.IsActive());
			}
		}

		if (Kortex::IPluginManager::HasInstance())
		{
			KxXMLNode pluginsNode = rootNode.NewElement("Plugins");
			for (const ProfilePlugin& plugin: m_Plugins)
			{
				KxXMLNode node = pluginsNode.NewElement("Entry");
				node.SetAttribute("Name", plugin.GetPluginName());
				node.SetAttribute("Enabled", plugin.IsActive());
			}
		}

		KxFileStream stream(GetOrderFile(), KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Read);
		xml.Save(stream);
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
		Save();
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
		Save();
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
