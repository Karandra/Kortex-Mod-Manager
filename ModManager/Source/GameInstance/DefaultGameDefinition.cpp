#include "pch.hpp"
#include "DefaultGameDefinition.h"
#include "Application/IApplication.h"
#include <kxf/General/IndexedEnum.h>
#include <kxf/System/Registry.h>

namespace
{
	constexpr struct
	{
		static constexpr kxf::XChar String[] = wxS("String");
		static constexpr kxf::XChar FSPath[] = wxS("FSPath");
		static constexpr kxf::XChar Integer[] = wxS("Integer");

		static constexpr kxf::XChar Registry[] = wxS("Registry");
	} g_TypeNames;

	constexpr struct
	{
		static constexpr kxf::XChar ID[] = wxS("ID");
		static constexpr kxf::XChar Source[] = wxS("Source");
	} g_AttributeNames;

	constexpr struct
	{
		static constexpr kxf::XChar Namespace[] = wxS("Definition");

		static constexpr kxf::XChar GameID[] = wxS("GameID");
		static constexpr kxf::XChar GameName[] = wxS("GameName");
		static constexpr kxf::XChar GameNameShort[] = wxS("GameNameShort");
		static constexpr kxf::XChar SortOrder[] = wxS("SortOrder");

		static constexpr kxf::XChar FileName[] = wxS("FileName");
		static constexpr kxf::XChar RootDirectory[] = wxS("RootDirectory");
		static constexpr kxf::XChar GameDirectory[] = wxS("GameDirectory");
	} g_DefinitionNames;

	constexpr struct
	{
		static constexpr kxf::XChar Namespace[] = wxS("Resource");

		static constexpr kxf::XChar Directory[] = wxS("Directory");
		static constexpr kxf::XChar Icon[] = wxS("Icon");
	} g_ResourceNames;

	struct NameToRegistryRoot final: public kxf::IndexedEnumDefinition<NameToRegistryRoot, kxf::RegistryRootKey, kxf::String>
	{
		inline static const TItem Items[] =
		{
			{kxf::RegistryRootKey::Users, wxS("HKEY_USERS")},
			{kxf::RegistryRootKey::ClassesRoot, wxS("HKEY_CLASSES_ROOT")},
			{kxf::RegistryRootKey::CurrentUser, wxS("HKEY_CURRENT_USER")},
			{kxf::RegistryRootKey::LocalMachine, wxS("HKEY_LOCAL_MACHINE")},
			{kxf::RegistryRootKey::CurrentConfig, wxS("HKEY_CURRENT_CONFIG")},
		};
	};
	struct NameToRegistryValueType final: public kxf::IndexedEnumDefinition<NameToRegistryValueType, kxf::RegistryValueType, kxf::String>
	{
		inline static const TItem Items[] =
		{
			{kxf::RegistryValueType::String, wxS("REG_VALUE_SZ")},
			{kxf::RegistryValueType::StringExpand, wxS("REG_VALUE_EXPAND_SZ")},
			{kxf::RegistryValueType::StringArray, wxS("REG_VALUE_MULTI_SZ")},
			{kxf::RegistryValueType::UInt32, wxS("REG_VALUE_DWORD")},
			{kxf::RegistryValueType::UInt64, wxS("REG_VALUE_QWORD")},
		};
	};

	std::optional<kxf::String> LoadRegistryValriable(const kxf::XMLNode& itemNode)
	{
		// 32 or 64 bit registry branch
		kxf::RegistryWOW64 regWOW64 = kxf::RegistryWOW64::Default;
		switch (itemNode.GetFirstChildElement("Branch").GetValueInt(0))
		{
			case 32:
			{
				regWOW64 = kxf::RegistryWOW64::Access32;
				break;
			}
			case 64:
			{
				regWOW64 = kxf::RegistryWOW64::Access64;
				break;
			}
		};

		// Main key
		if (auto rootKey = NameToRegistryRoot::TryFromString(itemNode.GetFirstChildElement("Root").GetValue()))
		{
			decltype(auto) app = Kortex::IApplication::GetInstance();

			kxf::FSPath path = app.ExpandVariables(itemNode.GetFirstChildElement("Path").GetValue());
			kxf::RegistryValueType type = NameToRegistryValueType::TryFromString(itemNode.GetFirstChildElement("Type").GetValue()).value_or(kxf::RegistryValueType::String);

			if (kxf::RegistryKey regKey(*rootKey, path, kxf::RegistryAccess::Read, regWOW64); regKey)
			{
				kxf::String name = app.ExpandVariables(itemNode.GetFirstChildElement("Name").GetValue());

				auto ConvertIntType = [](auto value) -> std::optional<kxf::String>
				{
					if (value)
					{
						return kxf::String::Format(wxS("%1"), *value);
					}
					return {};
				};

				switch (type)
				{
					case kxf::RegistryValueType::String:
					{
						return regKey.GetStringValue(name);
					}
					case kxf::RegistryValueType::StringExpand:
					{
						return regKey.GetStringExpandValue(name, true);
					}
					case kxf::RegistryValueType::StringArray:
					{
						kxf::String result;
						regKey.GetStringArrayValue(name, [&](kxf::String value)
						{
							if (!result.IsEmpty())
							{
								result += wxS('|');
							}
							result += std::move(value);

							return true;
						});

						if (!result.empty())
						{
							return result;
						}
						break;
					}
					case kxf::RegistryValueType::UInt32:
					{
						return ConvertIntType(regKey.GetUInt32Value(name));
					}
					case kxf::RegistryValueType::UInt64:
					{
						return ConvertIntType(regKey.GetUInt64Value(name));
					}
				};
			}
		}
		return {};
	}
}

namespace Kortex
{
	void DefaultGameDefinition::MakeNull()
	{
		m_GameID = {};
		m_DefitionData.ClearChildren();
	}
	kxf::FSPath DefaultGameDefinition::GetDefinitionFileName() const
	{
		if (!IsNull())
		{
			return kxf::FSPath(m_GameID) + wxS(".xml");
		}
		return {};
	}

	bool DefaultGameDefinition::LoadDefinition()
	{
		const auto rootNode = m_DefitionData.QueryElement("Definition");

		if (m_GameID = rootNode.GetAttribute("ID"))
		{
			SetupVariables(rootNode.QueryElement("Variables"));
			return true;
		}
		return false;
	}
	void DefaultGameDefinition::SetupVariables(const kxf::XMLNode& variablesRoot)
	{
		// Basic variables
		m_Variables.SetItem(g_DefinitionNames.Namespace, g_DefinitionNames.FileName, GetDefinitionFileName());
		m_Variables.SetItem(g_DefinitionNames.Namespace, g_DefinitionNames.RootDirectory, m_RootFS.GetLookupDirectory());
		m_Variables.SetItem(g_DefinitionNames.Namespace, g_DefinitionNames.GameDirectory, m_GameFS.GetLookupDirectory());

		m_Variables.SetItem(g_DefinitionNames.Namespace, g_DefinitionNames.GameID, m_GameID.ToString());
		m_Variables.SetItem(g_DefinitionNames.Namespace, g_DefinitionNames.GameName, DefaultGameDefinition::GetGameName());
		m_Variables.SetItem(g_DefinitionNames.Namespace, g_DefinitionNames.GameNameShort, DefaultGameDefinition::GetGameShortName());
		m_Variables.SetItem(g_DefinitionNames.Namespace, g_DefinitionNames.SortOrder, m_SortOrder);

		m_Variables.SetItem(g_ResourceNames.Namespace, g_ResourceNames.Directory, m_ResourcesFS.GetLookupDirectory());

		// Variables from the XML
		variablesRoot.EnumChildElements([&](const kxf::XMLNode& itemNode)
		{
			if (auto id = itemNode.QueryAttribute(g_AttributeNames.ID))
			{
				// Default source
				auto value = itemNode.QueryValue();

				// Handle registry source
				if (!value && itemNode.GetAttribute(g_AttributeNames.Source) == g_TypeNames.Registry)
				{
					value = LoadRegistryValriable(itemNode);
				}

				// Process and store the value
				if (value)
				{
					kxf::String type = itemNode.GetAttribute("Type", g_TypeNames.String);
					kxf::String ns = itemNode.GetAttribute("Namespace", g_DefinitionNames.Namespace);

					if (type == g_TypeNames.String)
					{
						// Setup special values
						if (ns == g_DefinitionNames.Namespace)
						{
							if (*id == g_DefinitionNames.GameName)
							{
								m_GameName = *value;
							}
							else if (*id == g_DefinitionNames.GameNameShort)
							{
								m_GameNameShort = *value;
							}
						}
						m_Variables.SetItem(std::move(ns), std::move(*id), std::move(*value));
					}
					else if (type == g_TypeNames.FSPath)
					{
						// Update file system scopes
						if (ns == g_DefinitionNames.Namespace)
						{
							if (*id == g_DefinitionNames.GameDirectory)
							{
								m_GameFS.SetLookupDirectory(*value);
							}
						}
						else if (ns == g_ResourceNames.Namespace)
						{
							if (*id == g_ResourceNames.Directory)
							{
								m_ResourcesFS.SetLookupDirectory(*value);
							}
						}
						m_Variables.SetItem(std::move(ns), std::move(*id), kxf::FSPath(std::move(*value)));
					}
					else if (type == g_TypeNames.Integer)
					{
						if (auto valueInt = value->ToInt<int64_t>())
						{
							// Setup special values
							if (ns == g_DefinitionNames.Namespace)
							{
								if (*id == g_DefinitionNames.SortOrder)
								{
									m_SortOrder = static_cast<int>(*valueInt);
								}
							}
							m_Variables.SetItem(std::move(ns), std::move(*id), *valueInt);
						}
					}
				}
			}
			return true;
		});
	}

	// IGameDefinition
	bool DefaultGameDefinition::LoadDefinitionData(const kxf::IFileSystem& fileSystem)
	{
		if (!IsNull() || !fileSystem.IsLookupScoped())
		{
			return false;
		}

		if (m_RootFS = fileSystem.GetLookupDirectory())
		{
			m_ResourcesFS = m_RootFS.GetLookupDirectory();

			if (auto stream = fileSystem.OpenToRead("Definition.xml"); stream && m_DefitionData.Load(*stream))
			{
				if (LoadDefinition() && m_GameID == fileSystem.GetLookupDirectory().GetName())
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
			if (auto stream = m_RootFS.OpenToWrite(GetDefinitionFileName()); stream && m_DefitionData.Save(*stream))
			{
				// Reload definition to keep this object's data consistent with the XML on disk

				MakeNull();
				m_Variables.ClearItems();
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
