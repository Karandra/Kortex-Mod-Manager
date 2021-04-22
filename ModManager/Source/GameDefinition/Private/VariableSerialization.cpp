#include "pch.hpp"
#include "VariableSerialization.h"
#include "Application/IApplication.h"
#include "Application/IResourceManager.h"
#include <kxf/General/IndexedEnum.h>
#include <kxf/System/Registry.h>

namespace
{
	constexpr struct
	{
		static constexpr kxf::XChar String[] = wxS("String");
		static constexpr kxf::XChar FSPath[] = wxS("FSPath");
		static constexpr kxf::XChar URI[] = wxS("URI");
		static constexpr kxf::XChar ResourceID[] = wxS("ResourceID");
		static constexpr kxf::XChar Integer[] = wxS("Integer");

		static constexpr kxf::XChar Registry[] = wxS("Registry");
	} g_TypeNames;

	constexpr struct
	{
		static constexpr kxf::XChar Name[] = wxS("Name");
		static constexpr kxf::XChar Source[] = wxS("Source");
	} g_AttributeNames;

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

namespace Kortex::GameInstance::Private
{
	size_t VariableLoader::DoLoadVariables()
	{
		size_t count = 0;
		for (const kxf::XMLNode& itemNode: m_VariablesRoot.EnumChildElements(wxS("Item")))
		{
			if (auto name = itemNode.QueryAttribute(g_AttributeNames.Name))
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
					auto type = itemNode.QueryAttribute("Type");
					kxf::String ns = itemNode.QueryAttribute("Namespace").value_or(m_DefaultNamespace);

					if (!type || *type == g_TypeNames.String)
					{
						if (!m_OnString || m_OnString(*type, ns, *name, *value))
						{
							count++;
							m_Collection.SetItem(std::move(ns), std::move(*name), std::move(*value));
						}
					}
					else if (*type == g_TypeNames.FSPath)
					{
						kxf::FSPath fsPathValue = std::move(*value);
						if (!m_OnFSPath || m_OnFSPath(*type, ns, *name, fsPathValue))
						{
							count++;
							m_Collection.SetItem(std::move(ns), std::move(*name), std::move(fsPathValue));
						}
					}
					else if (*type == g_TypeNames.URI)
					{
						kxf::URI uriValue = std::move(*value);
						if (!m_OnURI || m_OnURI(*type, ns, *name, uriValue))
						{
							count++;
							m_Collection.SetItem(std::move(ns), std::move(*name), std::move(uriValue));
						}
					}
					else if (*type == g_TypeNames.ResourceID)
					{
						kxf::ResourceID resValue = IResourceManager::MakeResourceIDWithCategory(m_ResourcesCategory, std::move(*value));
						if (!m_OnResourceID || m_OnResourceID(*type, ns, *name, resValue))
						{
							count++;
							m_Collection.SetItem(std::move(ns), std::move(*name), std::move(resValue));
						}
					}
					else if (*type == g_TypeNames.Integer)
					{
						if (auto valueInt = value->ToInt<int64_t>())
						{
							if (!m_OnInteger || m_OnInteger(*type, ns, *name, *valueInt))
							{
								count++;
								m_Collection.SetItem(std::move(ns), std::move(*name), *valueInt);
							}
						}
					}
				}
			}
		}
		return count;
	}
}

namespace Kortex::GameInstance::Private
{
	size_t VariableSaver::DoSaveVariables()
	{
		return 0;
	}
}
