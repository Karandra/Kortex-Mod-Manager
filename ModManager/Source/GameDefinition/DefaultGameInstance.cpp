#include "pch.hpp"
#include "DefaultGameInstance.h"
#include "DefaultGameProfile.h"
#include "IGameDefinition.h"
#include "Application/IApplication.h"
#include "Private/VariableSerialization.h"

namespace
{
	constexpr struct
	{
		static constexpr kxf::XChar Namespace[] = kxS("Instance");
		static constexpr kxf::XChar XMLFileName[] = kxS("Instance.xml");

		static constexpr kxf::XChar Name[] = kxS("Name");
		static constexpr kxf::XChar DefinitionName[] = kxS("DefinitionName");

		static constexpr kxf::XChar GameDirectory[] = kxS("GameDirectory");
		static constexpr kxf::XChar ModsDirectory[] = kxS("ModsDirectory");
		static constexpr kxf::XChar ProfilesDirectory[] = kxS("ProfilesDirectory");
		static constexpr kxf::XChar DownloadsDirectory[] = kxS("DownloadsDirectory");
		static constexpr kxf::XChar MountedGame[] = kxS("MountedGame");
	} g_InstanceNames;

	constexpr struct
	{
		static constexpr kxf::XChar Namespace[] = kxS("ActiveProfile");

		static constexpr kxf::XChar RootDirectory[] = kxS("RootDirectory");
		static constexpr kxf::XChar GameSavesDirectory[] = kxS("GameSavesDirectory");
		static constexpr kxf::XChar GameConfigDirectory[] = kxS("GameConfigDirectory");
		static constexpr kxf::XChar WriteTargetDirectory[] = kxS("WriteTargetDirectory");
	} g_ActiveProfileNames;

	constexpr struct
	{
		static constexpr kxf::XChar Active[] = kxS("Active");
		static constexpr kxf::XChar GameProfiles[] = kxS("GameProfiles");
	} g_OptionNames;
}

namespace Kortex
{
	void DefaultGameInstance::MakeNull()
	{
		m_Variables.ClearItems();
		m_InstanceData.ClearChildren();
		m_RootFS = {};
		m_Name = kxf::NullString;
	}

	bool DefaultGameInstance::LoadInstance()
	{
		const auto rootNode = m_InstanceData.QueryElement("Instance");
		if (!m_Name.IsEmpty() && ResolveDefinition(rootNode.GetAttribute("ParentDefinition")))
		{
			SetupVariables(rootNode.QueryElement("Variables"));
			return true;
		}
		return false;
	}
	bool DefaultGameInstance::ResolveDefinition(const kxf::String& name)
	{
		m_Definition = IApplication::GetInstance().GetGameDefinitionByName(name);
		return m_Definition != nullptr;
	}
	void DefaultGameInstance::SetupVariables(const kxf::XMLNode& variablesRoot)
	{
		// Set basic variables
		m_Variables.SetItem(g_InstanceNames.Namespace, g_InstanceNames.Name, m_Name);
		m_Variables.SetItem(g_InstanceNames.Namespace, g_InstanceNames.DefinitionName, m_Definition->GetName());

		// Set dynamic variables
		auto AddActiveProfileFSPath = [&](IGameProfile::Location locationID, const auto& name)
		{
			m_Variables.SetDynamicItem(g_ActiveProfileNames.Namespace, name, [this, locationID]() -> kxf::Any
			{
				if (m_ActiveProfile)
				{
					return m_ActiveProfile->GetFileSystem(locationID).GetLookupDirectory();
				}
				return {};
			});
		};
		AddActiveProfileFSPath(IGameProfile::Location::Root, g_ActiveProfileNames.RootDirectory);
		AddActiveProfileFSPath(IGameProfile::Location::GameSaves, g_ActiveProfileNames.GameSavesDirectory);
		AddActiveProfileFSPath(IGameProfile::Location::GameConfig, g_ActiveProfileNames.GameConfigDirectory);
		AddActiveProfileFSPath(IGameProfile::Location::WriteTarget, g_ActiveProfileNames.WriteTargetDirectory);

		// Load variables from the XML
		GameInstance::Private::VariableLoader loader(m_Variables, variablesRoot, g_InstanceNames.Namespace, m_Definition->GetName() + kxS('/') + m_Name);
		loader.OnFSPath([&](const kxf::String& type, const kxf::String& ns, const kxf::String& name, const kxf::FSPath& value)
		{
			if (ns == g_InstanceNames.Namespace)
			{
				if (name == g_InstanceNames.ModsDirectory)
				{
					m_ModsFS.SetLookupDirectory(value);
				}
				else if (name == g_InstanceNames.ProfilesDirectory)
				{
					m_ProfilesFS.SetLookupDirectory(value);
				}
				else if (name == g_InstanceNames.DownloadsDirectory)
				{
					m_DownloadsFS.SetLookupDirectory(value);
				}
				else if (name == g_InstanceNames.GameDirectory)
				{
					m_GameFS.SetLookupDirectory(value);
				}
				else if (name == g_InstanceNames.MountedGame)
				{
					m_MountedGameFS.SetLookupDirectory(value);
				}
			}
			return true;
		});

		loader.Invoke();
	}
	void DefaultGameInstance::LoadProfiles()
	{
		auto option = ReadInstanceOption(g_OptionNames.GameProfiles);
		const kxf::String activeName = option.GetAttribute(g_OptionNames.Active);

		for (const kxf::FileItem& item: m_ProfilesFS.EnumItems({}, {}, kxf::FSActionFlag::LimitToDirectories))
		{
			if (item.IsNormalItem())
			{
				auto profile = std::make_unique<DefaultGameProfile>();

				kxf::ScopedNativeFileSystem fs(item.GetFullPath());
				if (profile->LoadProfileData(*this, fs))
				{
					if (!m_ActiveProfile && profile->GetName() == activeName)
					{
						m_ActiveProfile = profile.get();
					}
					m_Profiles.emplace_back(std::move(profile));
				}
			}
		}
	}

	// IGameDefinition
	bool DefaultGameInstance::IsNull() const
	{
		return m_Definition == nullptr || m_Definition->IsNull() || m_InstanceData.IsNull() || m_Name.IsEmpty() || m_RootFS.IsNull();
	}
	kxf::IFileSystem& DefaultGameInstance::GetFileSystem(Location locationID)
	{
		switch (locationID)
		{
			case Location::Root:
			{
				return m_RootFS;
			}
			case Location::Game:
			{
				return m_GameFS;
			}
			case Location::Profiles:
			{
				return m_ProfilesFS;
			}
			case Location::Mods:
			{
				return m_ModsFS;
			}
			case Location::Downloads:
			{
				return m_DownloadsFS;
			}
			case Location::MountedGame:
			{
				return m_MountedGameFS;
			}
		};
		return kxf::FileSystem::GetNullFileSystem();
	}

	// IGameInstance
	bool DefaultGameInstance::LoadInstanceData(const kxf::IFileSystem& fileSystem)
	{
		if (!IsNull() || !fileSystem.IsLookupScoped() || !fileSystem)
		{
			return false;
		}
		
		if (m_RootFS = fileSystem.GetLookupDirectory())
		{
			m_ModsFS = m_RootFS.GetLookupDirectory() / kxS("Mods");
			m_ProfilesFS = m_RootFS.GetLookupDirectory() / kxS("Profiles");
			m_DownloadsFS = m_RootFS.GetLookupDirectory() / kxS("Downloads");
			m_MountedGameFS = m_RootFS.GetLookupDirectory() / kxS("MountedGame");

			if (auto stream = fileSystem.OpenToRead(g_InstanceNames.XMLFileName); stream && m_InstanceData.Load(*stream))
			{
				m_Name = fileSystem.GetLookupDirectory().GetName();
				if (LoadInstance())
				{
					LoadProfiles();
					return true;
				}
				else
				{
					MakeNull();
				}
			}
		}
		return false;
	}
	bool DefaultGameInstance::SaveInstanceData()
	{
		if (!IsNull())
		{
			GameInstance::Private::VariableSaver saver(m_Variables, m_InstanceData.ConstructElement("Variables"));
			saver.Invoke();

			if (auto stream = m_RootFS.OpenToWrite(g_InstanceNames.XMLFileName); stream && m_InstanceData.Save(*stream))
			{
				// Reload instance to keep this object's data consistent with the XML on disk

				MakeNull();
				return LoadInstance();
			}
		}
		return false;
	}

	size_t DefaultGameInstance::EnumProfiles(std::function<bool(IGameProfile& profile)> func)
	{
		if (func)
		{
			size_t count = 0;
			for (const auto& profile: m_Profiles)
			{
				count++;
				if (!std::invoke(func, *profile))
				{
					break;
				}
			}
			return count;
		}
		else
		{
			return m_Profiles.size();
		}
	}

	IGameProfile* DefaultGameInstance::CreateProfile(const kxf::String& profileName, const IGameProfile* baseProfile)
	{
		return nullptr;
	}
	bool DefaultGameInstance::RemoveProfile(IGameProfile& profile)
	{
		return false;
	}
	bool DefaultGameInstance::RenameProfile(IGameProfile& profile, const kxf::String& newName)
	{
		return false;
	}
	bool DefaultGameInstance::SwitchActiveProfile(IGameProfile& profile)
	{
		return false;
	}
}
