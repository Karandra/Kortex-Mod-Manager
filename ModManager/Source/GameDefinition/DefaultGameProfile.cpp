#include "pch.hpp"
#include "DefaultGameProfile.h"

namespace
{
	constexpr struct
	{
		static constexpr kxf::XChar XMLFileName[] = wxS("Profile.xml");
	} g_ProfileNames;
}

namespace Kortex
{
	bool DefaultGameProfile::LoadProfile()
	{
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
				return true;
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
	size_t DefaultGameProfile::EnumMods(std::function<bool(const GameProfileMod& gameMod)> func) const
	{
		if (func)
		{
			size_t count = 0;
			for (const auto& item: m_Mods)
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
			return m_Mods.size();
		}
	}
	size_t DefaultGameProfile::EnumPlugins(std::function<bool(const GameProfilePlugin& gamePlugin)> func) const
	{
		if (func)
		{
			size_t count = 0;
			for (const auto& item: m_Plugins)
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
			return m_Plugins.size();
		}
	}
}
