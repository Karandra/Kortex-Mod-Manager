#include "pch.hpp"
#include "DefaultGameDefinition.h"
#include "Private/VariableSerialization.h"

namespace
{
	constexpr struct
	{
		static constexpr kxf::XChar Namespace[] = wxS("Definition");
		static constexpr kxf::XChar XMLFileName[] = wxS("Definition.xml");

		static constexpr kxf::XChar Name[] = wxS("Name");
		static constexpr kxf::XChar GameName[] = wxS("GameName");
		static constexpr kxf::XChar GameNameShort[] = wxS("GameNameShort");
		static constexpr kxf::XChar SortOrder[] = wxS("SortOrder");

		static constexpr kxf::XChar RootDirectory[] = wxS("RootDirectory");
		static constexpr kxf::XChar GameDirectory[] = wxS("GameDirectory");
	} g_DefinitionNames;

	constexpr struct
	{
		static constexpr kxf::XChar Namespace[] = wxS("Resource");

		static constexpr kxf::XChar Directory[] = wxS("Directory");
		static constexpr kxf::XChar Icon[] = wxS("Icon");
	} g_ResourceNames;
}

namespace Kortex
{
	void DefaultGameDefinition::MakeNull()
	{
		m_Variables.ClearItems();
		m_Name = kxf::NullString;
		m_DefitionData.ClearChildren();
	}

	bool DefaultGameDefinition::LoadDefinition()
	{
		const auto rootNode = m_DefitionData.QueryElement("Definition");

		m_Name = rootNode.GetAttribute("Name");
		if (!m_Name.IsEmpty())
		{
			SetupVariables(rootNode.QueryElement("Variables"));
			return true;
		}
		return false;
	}
	void DefaultGameDefinition::SetupVariables(const kxf::XMLNode& variablesRoot)
	{
		// Set basic variables
		m_Variables.SetItem(g_DefinitionNames.Namespace, g_DefinitionNames.Name, m_Name);
		m_Variables.SetItem(g_DefinitionNames.Namespace, g_DefinitionNames.GameName, DefaultGameDefinition::GetGameName());
		m_Variables.SetItem(g_DefinitionNames.Namespace, g_DefinitionNames.GameNameShort, DefaultGameDefinition::GetGameShortName());
		m_Variables.SetItem(g_DefinitionNames.Namespace, g_DefinitionNames.SortOrder, m_SortOrder);

		m_Variables.SetItem(g_DefinitionNames.Namespace, g_DefinitionNames.RootDirectory, m_RootFS.GetLookupDirectory());
		m_Variables.SetItem(g_DefinitionNames.Namespace, g_DefinitionNames.GameDirectory, m_GameFS.GetLookupDirectory());

		m_Variables.SetItem(g_ResourceNames.Namespace, g_ResourceNames.Directory, m_ResourcesFS.GetLookupDirectory());

		// Load variables from the XML
		GameInstance::Private::VariableLoader loader(m_Variables, variablesRoot, g_DefinitionNames.Namespace);
		loader.OnString([&](const kxf::String& type, const kxf::String& ns, const kxf::String& name, const kxf::String& value)
		{
			if (ns == g_DefinitionNames.Namespace)
			{
				if (name == g_DefinitionNames.GameName)
				{
					m_GameName = value;
				}
				else if (name == g_DefinitionNames.GameNameShort)
				{
					m_GameNameShort = value;
				}
			}
			return true;
		});
		loader.OnFSPath([&](const kxf::String& type, const kxf::String& ns, const kxf::String& name, const kxf::FSPath& value)
		{
			// Update file system scopes
			if (ns == g_DefinitionNames.Namespace)
			{
				if (name == g_DefinitionNames.GameDirectory)
				{
					m_GameFS.SetLookupDirectory(value);
				}
			}
			else if (ns == g_ResourceNames.Namespace)
			{
				if (name == g_ResourceNames.Directory)
				{
					m_ResourcesFS.SetLookupDirectory(value);
				}
			}
			return true;
		});
		loader.OnInteger([&](const kxf::String& type, const kxf::String& ns, const kxf::String& name, int64_t value)
		{
			if (ns == g_DefinitionNames.Namespace)
			{
				if (name == g_DefinitionNames.SortOrder)
				{
					m_SortOrder = static_cast<int>(value);
				}
			}
			return true;
		});

		loader.Invoke();
	}

	// IGameDefinition
	bool DefaultGameDefinition::LoadDefinitionData(const kxf::IFileSystem& fileSystem)
	{
		if (!IsNull() || !fileSystem.IsLookupScoped() || !fileSystem)
		{
			return false;
		}

		if (m_RootFS = fileSystem.GetLookupDirectory())
		{
			m_ResourcesFS = m_RootFS.GetLookupDirectory();

			if (auto stream = fileSystem.OpenToRead(g_DefinitionNames.XMLFileName); stream && m_DefitionData.Load(*stream))
			{
				if (LoadDefinition() && m_Name == fileSystem.GetLookupDirectory().GetName())
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
	bool DefaultGameDefinition::SaveDefinitionData()
	{
		if (!IsNull())
		{
			GameInstance::Private::VariableSaver saver(m_Variables, m_DefitionData.ConstructElement("Variables"));
			saver.Invoke();

			if (auto stream = m_RootFS.OpenToWrite(g_DefinitionNames.XMLFileName); stream && m_DefitionData.Save(*stream))
			{
				// Reload definition to keep this object's data consistent with the XML on disk

				MakeNull();
				return LoadDefinition();
			}
		}
		return false;
	}

	const kxf::IImage2D& DefaultGameDefinition::GetIcon() const
	{
		if (!m_Icon)
		{
			m_Icon = IGameDefinition::LoadIcon(m_ResourcesFS, m_Variables.GetItem(g_ResourceNames.Namespace, g_ResourceNames.Icon).GetAs<kxf::FSPath>());
			if (!m_Icon)
			{
				m_Icon = IGameDefinition::GetGenericIcon();
			}
		}
		return *m_Icon;
	}
	kxf::IFileSystem& DefaultGameDefinition::GetFileSystem(Location locationID)
	{
		switch (locationID)
		{
			case Location::Root:
			{
				return m_RootFS;
			}
			case Location::Resources:
			{
				return m_ResourcesFS;
			}
			case Location::Game:
			{
				return m_GameFS;
			}
		};
		return kxf::FileSystem::GetNullFileSystem();
	}
}
