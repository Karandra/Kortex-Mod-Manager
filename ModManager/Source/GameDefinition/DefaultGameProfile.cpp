#include "pch.hpp"
#include "DefaultGameProfile.h"

namespace
{
	constexpr struct
	{
		static constexpr kxf::XChar XMLFileName[] = wxS("Profile.xml");

		static constexpr kxf::XChar Item[] = wxS("Item");
	} g_ProfileNames;

	constexpr struct
	{
		static constexpr kxf::XChar Root[] = wxS("GameMods");
		static constexpr kxf::XChar ItemsRoot[] = wxS("GameMods/Items");
	} g_GameModNames;

	constexpr struct
	{
		static constexpr kxf::XChar Root[] = wxS("GamePlugins");
		static constexpr kxf::XChar ItemsRoot[] = wxS("GamePlugins/Items");
	} g_GamePluginNames;

	constexpr struct
	{
		static constexpr kxf::XChar Name[] = wxS("Name");
		static constexpr kxf::XChar Signature[] = wxS("Signature");
		static constexpr kxf::XChar Active[] = wxS("Active");
	} g_AttributeNames;
}

namespace Kortex
{
	bool DefaultGameProfile::LoadProfile()
	{
		kxf::XMLNode rootNode = m_ProfileData.QueryElement("Profile");

		// Load mods
		m_GameMods.clear();
		rootNode.QueryElement(g_GameModNames.ItemsRoot).EnumChildElements([&, priority = 0](kxf::XMLNode node) mutable
		{
			if (!m_GameMods.emplace_back(node.GetAttribute(g_AttributeNames.Signature), node.GetAttributeBool(g_AttributeNames.Active), priority))
			{
				m_GameMods.pop_back();
			}
			priority++;

			return true;
		}, g_ProfileNames.Item);

		// Load plugins
		m_GamePlugins.clear();
		rootNode.QueryElement(g_GamePluginNames.ItemsRoot).EnumChildElements([&, priority = 0](kxf::XMLNode node) mutable
		{
			if (!m_GamePlugins.emplace_back(node.GetAttribute(g_AttributeNames.Name), node.GetAttributeBool(g_AttributeNames.Active), priority))
			{
				m_GamePlugins.pop_back();
			}
			priority++;

			return true;
		}, g_ProfileNames.Item);

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
			m_GameSavesFS = m_RootFS.GetLookupDirectory() / wxS("GameSaves");
			m_GameConfigFS = m_RootFS.GetLookupDirectory() / wxS("GameConfig");
			m_WriteTargetFS = m_RootFS.GetLookupDirectory() / wxS("WriteTarget");

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

	kxf::IFileSystem& DefaultGameProfile::GetLocation(Location locationID)
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
		
	}
	size_t DefaultGameProfile::EnumGameMods(std::function<bool(const GameProfileMod& gameMod)> func) const
	{
		if (func)
		{
			size_t count = 0;
			for (const auto& item: m_GameMods)
			{
				count++;
				if (!std::invoke(func, item))
				{
					break;
				}
			}
			return count;
		}
		else
		{
			return m_GameMods.size();
		}
	}
	size_t DefaultGameProfile::EnumGamePlugins(std::function<bool(const GameProfilePlugin& gamePlugin)> func) const
	{
		if (func)
		{
			size_t count = 0;
			for (const auto& item: m_GamePlugins)
			{
				count++;
				if (!std::invoke(func, item))
				{
					break;
				}
			}
			return count;
		}
		else
		{
			return m_GamePlugins.size();
		}
	}
}
