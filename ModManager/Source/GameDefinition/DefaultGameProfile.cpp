#include "pch.hpp"
#include "DefaultGameProfile.h"
#include "Application/IApplication.h"
#include "Modules/GameModManager/IGameModManager.h"
#include <kxf/Utility/Enumerator.h>

namespace
{
	constexpr struct
	{
		static constexpr kxf::XChar XMLFileName[] = kxS("Profile.xml");

		static constexpr kxf::XChar Item[] = kxS("Item");
	} g_ProfileNames;

	constexpr struct
	{
		static constexpr kxf::XChar Root[] = kxS("GameMods");
		static constexpr kxf::XChar ItemsRoot[] = kxS("GameMods/Items");
	} g_GameModNames;

	constexpr struct
	{
		static constexpr kxf::XChar Root[] = kxS("GamePlugins");
		static constexpr kxf::XChar ItemsRoot[] = kxS("GamePlugins/Items");
	} g_GamePluginNames;

	constexpr struct
	{
		static constexpr kxf::XChar Name[] = kxS("Name");
		static constexpr kxf::XChar Signature[] = kxS("Signature");
		static constexpr kxf::XChar Active[] = kxS("Active");
	} g_AttributeNames;
}

namespace Kortex
{
	bool DefaultGameProfile::LoadProfile()
	{
		kxf::XMLNode rootNode = m_ProfileData.QueryElement("Profile");

		// Load mods
		m_GameMods.clear();
		{
			int priority = 0;
			for (const kxf::XMLNode& node: rootNode.QueryElement(g_GameModNames.ItemsRoot).EnumChildElements(g_ProfileNames.Item))
			{
				if (!m_GameMods.emplace_back(node.GetAttribute(g_AttributeNames.Signature), node.GetAttributeBool(g_AttributeNames.Active), priority))
				{
					m_GameMods.pop_back();
				}
				priority++;

				return true;
			}
		}

		// Load plugins
		m_GamePlugins.clear();
		{
			int priority = 0;
			for (const kxf::XMLNode& node: rootNode.QueryElement(g_GamePluginNames.ItemsRoot).EnumChildElements(g_ProfileNames.Item))
			{
				if (!m_GamePlugins.emplace_back(node.GetAttribute(g_AttributeNames.Name), node.GetAttributeBool(g_AttributeNames.Active), priority))
				{
					m_GamePlugins.pop_back();
				}
				priority++;

				return true;
			}
		}
		return true;
	}
	bool DefaultGameProfile::SaveProfile() const
	{
		// Save mods
		kxf::XMLNode modsNode = m_ProfileData.ConstructElement(g_GameModNames.ItemsRoot);
		modsNode.ClearChildren();

		for (const auto& item: m_GameMods)
		{
			auto node = modsNode.NewElement(g_ProfileNames.Item);
			node.SetAttribute(g_AttributeNames.Signature, item.GetSignature());
			node.SetAttribute(g_AttributeNames.Active, item.IsActive());
		}

		// Save plugins
		kxf::XMLNode pluginsNode = m_ProfileData.ConstructElement(g_GamePluginNames.ItemsRoot);
		pluginsNode.ClearChildren();

		for (const auto& item: m_GamePlugins)
		{
			auto node = pluginsNode.NewElement(g_ProfileNames.Item);
			node.SetAttribute(g_AttributeNames.Name, item.GetName());
			node.SetAttribute(g_AttributeNames.Active, item.IsActive());
		}

		return true;
	}

	// IGameProfile
	bool DefaultGameProfile::LoadProfileData(IGameInstance& owningIntsance, const kxf::IFileSystem& fileSystem)
	{
		if (!IsNull() || !owningIntsance || !fileSystem || !fileSystem.IsLookupScoped())
		{
			return false;
		}

		if (m_RootFS = fileSystem.GetLookupDirectory())
		{
			m_GameSavesFS = m_RootFS.GetLookupDirectory() / kxS("GameSaves");
			m_GameConfigFS = m_RootFS.GetLookupDirectory() / kxS("GameConfig");
			m_WriteTargetFS = m_RootFS.GetLookupDirectory() / kxS("WriteTarget");

			if (auto stream = fileSystem.OpenToRead(g_ProfileNames.XMLFileName); stream && m_ProfileData.Load(*stream))
			{
				m_OwningInstance = &owningIntsance;
				m_Name = fileSystem.GetLookupDirectory().GetName();

				return LoadProfile();
			}
		}
		return false;
	}
	bool DefaultGameProfile::SaveProfileData() const
	{
		if (!IsNull())
		{
			if (auto stream = m_RootFS.OpenToWrite(g_ProfileNames.XMLFileName); stream && m_ProfileData.Save(*stream))
			{
				return SaveProfile();
			}
		}
		return false;
	}

	kxf::IFileSystem& DefaultGameProfile::GetFileSystem(Location locationID)
	{
		switch (locationID)
		{
			case IGameProfile::Location::Root:
			{
				return m_RootFS;
			}
			case IGameProfile::Location::GameSaves:
			{
				return m_GameSavesFS;
			}
			case IGameProfile::Location::GameConfig:
			{
				return m_GameConfigFS;
			}
			case IGameProfile::Location::WriteTarget:
			{
				return m_WriteTargetFS;
			}
		};
		return kxf::FileSystem::GetNullFileSystem();
	}

	void DefaultGameProfile::SyncWithCurrentState()
	{
		m_GameMods.clear();
		if (auto manager = IApplication::GetInstance().GetModule<IGameModManager>())
		{
			for (const IGameMod& mod: manager->EnumMods())
			{
				m_GameMods.emplace_back(mod);
			}
		}

		m_GamePlugins.clear();
		// if () ...
	}
	kxf::Enumerator<const GameProfileMod&> DefaultGameProfile::EnumGameMods() const
	{
		return kxf::Utility::EnumerateIndexableContainer<const GameProfileMod&>(m_GameMods);
	}
	kxf::Enumerator<const GameProfilePlugin&> DefaultGameProfile::EnumGamePlugins() const
	{
		return kxf::Utility::EnumerateIndexableContainer<const GameProfilePlugin&>(m_GamePlugins);
	}
}
