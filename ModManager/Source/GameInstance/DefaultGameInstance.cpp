#include "pch.hpp"
#include "DefaultGameInstance.h"
#include "IGameDefinition.h"
#include "Application/IApplication.h"
#include "Private/VariableSerialization.h"

namespace
{
	constexpr struct
	{
		static constexpr kxf::XChar Namespace[] = wxS("Instance");
		static constexpr kxf::XChar XMLFileName[] = wxS("Instance.xml");

		static constexpr kxf::XChar Name[] = wxS("Name");
		static constexpr kxf::XChar DefinitionName[] = wxS("DefinitionName");

		static constexpr kxf::XChar GameDirectory[] = wxS("GameDirectory");
		static constexpr kxf::XChar ModsDirectory[] = wxS("ModsDirectory");
		static constexpr kxf::XChar ProfilesDirectory[] = wxS("ProfilesDirectory");
		static constexpr kxf::XChar DownloadsDirectory[] = wxS("DownloadsDirectory");
		static constexpr kxf::XChar MountedGame[] = wxS("MountedGame");
	} g_InstanceNames;
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
		m_Definition = IApplication::GetInstance().FindGameDefinition(name);
		return m_Definition != nullptr;
	}
	void DefaultGameInstance::SetupVariables(const kxf::XMLNode& variablesRoot)
	{
		// Set basic variables
		m_Variables.SetItem(g_InstanceNames.Namespace, g_InstanceNames.Name, m_Name);
		m_Variables.SetItem(g_InstanceNames.Namespace, g_InstanceNames.DefinitionName, m_Definition->GetName());

		// Load variables from the XML
		GameInstance::Private::VariableLoader loader(m_Variables, variablesRoot, g_InstanceNames.Namespace);
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
			m_ModsFS = m_RootFS.GetLookupDirectory() / wxS("Mods");
			m_ProfilesFS = m_RootFS.GetLookupDirectory() / wxS("Profiles");
			m_DownloadsFS = m_RootFS.GetLookupDirectory() / wxS("Downloads");
			m_MountedGameFS = m_RootFS.GetLookupDirectory() / wxS("MountedGame");

			if (auto stream = fileSystem.OpenToRead(g_InstanceNames.XMLFileName); stream && m_InstanceData.Load(*stream))
			{
				m_Name = fileSystem.GetLookupDirectory().GetName();
				if (LoadInstance())
				{
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

	std::unique_ptr<IGameProfile> DefaultGameInstance::CreateProfile(const kxf::String& profileName, const IGameProfile* baseProfile, kxf::FlagSet<CopyFlag> copyFlags)
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
