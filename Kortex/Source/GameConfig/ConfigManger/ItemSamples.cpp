#include "stdafx.h"
#include "ItemSamples.h"
#include "Item.h"
#include "Items/SimpleItem.h"
#include "GameConfig/IConfigManager.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxXML.h>
#include <KxFramework/KxSystem.h>
#include <KxFramework/KxSystemSettings.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxFileFinder.h>

namespace
{
	using Kortex::GameConfig::SortOptionsID;
	using Kortex::GameConfig::SortOptionsValue;

	enum class CompareResult
	{
		LessThan = 1,
		Equal = 2,
		GreaterThan = 3
	};
	class CompareStringsData
	{
		private:
			NLSVERSIONINFOEX m_VersionInfo = {0};
			bool m_OK = false;

		public:
			CompareStringsData(const wchar_t* localeName)
			{
				if (KxSystem::IsWindows8OrGreater())
				{
					m_OK = ::GetNLSVersionEx(SYSNLS_FUNCTION::COMPARE_STRING, localeName, &m_VersionInfo);
				}
			}

		public:
			NLSVERSIONINFOEX* GetVersionInfo()
			{
				return m_OK ? &m_VersionInfo : nullptr;
			}
	};
	CompareResult CompareStrings(const wxString& v1, const wxString& v2, SortOptionsValue sortOptions)
	{
		constexpr const auto localeName = LOCALE_NAME_INVARIANT;
		static CompareStringsData compareData(localeName);

		DWORD options = 0;
		if (sortOptions.HasFlag(SortOptionsID::IgnoreCase))
		{
			options |= NORM_IGNORECASE;
		}
		if (sortOptions.HasFlag(SortOptionsID::DigitsAsNumbers))
		{
			options |= SORT_DIGITSASNUMBERS;
		}

		NLSVERSIONINFO* versionInfo = reinterpret_cast<NLSVERSIONINFO*>(compareData.GetVersionInfo());
		const int ret = ::CompareStringEx(localeName, options, v1.wc_str(), v1.length(), v2.wc_str(), v2.length(), versionInfo, nullptr, 0);
		return static_cast<CompareResult>(ret);
	}
}

namespace Kortex::GameConfig
{
	size_t ItemSamples::LoadImmediateItems(const KxXMLNode& rootNode)
	{
		size_t counter = 0;
		for (KxXMLNode node = rootNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{
			SampleValue& sample = m_Values.emplace_back(m_Item.GetManager().TranslateItemLabel(node, {}, wxS("Samples")));
			sample.GetValue().Deserialize(node.GetValue(), m_Item);

			counter++;
		}
		return counter;
	}
	void ItemSamples::SortImmediateItems()
	{
		if (!m_SortOrder.IsDefault())
		{
			auto Comparator = [this](const SampleValue& v1, const SampleValue& v2)
			{
				const CompareResult ret = CompareStrings(v1.GetValue().As<wxString>(), v2.GetValue().As<wxString>(), m_SortOptions);
				switch (m_SortOrder.GetValue())
				{
					case SortOrderID::Ascending:
					{
						return ret == CompareResult::LessThan;
					}
					case SortOrderID::Descending:
					{
						return ret != CompareResult::LessThan;
					}
				};
				return false;
			};
			std::sort(m_Values.begin(), m_Values.end(), Comparator);
		}
	}
	void ItemSamples::GenerateItems(const ItemValue::Vector& arguments)
	{
		switch (m_SampligFunction.GetValue())
		{
			case SamplingFunctionID::GetVideoAdapters:
			{
				for (const auto& adapter: KxSystemSettings::EnumVideoAdapters())
				{
					m_Values.emplace_back(adapter.DeviceName, adapter.DeviceString);
				}
				break;
			}
			case SamplingFunctionID::GetVideoModes:
			{
				std::unordered_set<wxString> hashMap;
				for (const auto& videoMode: KxSystemSettings::EnumVideoModes(wxEmptyString))
				{
					wxString hashValue = KxString::Format(wxS("%1×%2"), videoMode.Width, videoMode.Height);
					if (hashMap.emplace(hashValue).second)
					{
						m_Values.emplace_back(hashValue);
					}
				}
				break;
			}
			case SamplingFunctionID::GetVirtualKeys:
			{
				for (const auto&[keyCode, info]: LoadVirtualKeys())
				{
					m_Values.emplace_back(info.Name, keyCode);
				}
				break;
			}
			case SamplingFunctionID::FindFiles:
			{
				if (arguments.size() > 1)
				{
					KxFileFinder finder(arguments[0].As<wxString>());
					for (KxFileItem item = finder.FindNext(); item.IsOK(); item = finder.FindNext())
					{
						if (item.IsFile() && item.IsNormalItem())
						{
							m_Values.emplace_back(item.GetName());
						}
					}
				}
				break;
			}
		};
	}
	const ItemSamples::VirtualKeyInfo::Map& ItemSamples::LoadVirtualKeys()
	{
		static VirtualKeyInfo::Map virtualKeys;

		if (virtualKeys.empty())
		{
			KxFileStream xmlStream(IApplication::GetInstance()->GetDataFolder() + wxS("VirtualKeys.xml"), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting);
			KxXMLDocument xml(xmlStream);

			const ITranslator& translator = m_Item.GetManager().GetTranslator();
			KxXMLNode node = xml.QueryElement("VirtualKeys");
			for (node = node.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
			{
				unsigned long keyCode = WXK_NONE;
				wxString value = node.GetValue();
				if (value.Mid(2).ToCULong(&keyCode, 16) || value.ToCULong(&keyCode, 16))
				{
					wxString vkid = node.GetAttribute("VKID");
					wxString name = node.GetAttribute("Name");
					if (name.IsEmpty())
					{
						name = KxString::Format("%1", keyCode);
					}
					auto label = translator.TryGetString(wxS("ConfigManager.VirtualKey.") + vkid);
					if (label)
					{
						name = *label;
					}

					virtualKeys.insert_or_assign(keyCode, VirtualKeyInfo {vkid, name, keyCode});
				}
			}

			if (!virtualKeys.count(WXK_NONE))
			{
				virtualKeys.insert_or_assign(WXK_NONE, VirtualKeyInfo {"VK_NONE", translator.GetString("ConfigManager.VirtualKey.VK_NONE"), WXK_NONE});
			}
		}
		return virtualKeys;
	}

	ItemSamples::ItemSamples(Item& item, const KxXMLNode& samplesNode)
		:m_Item(item)
	{
		Load(samplesNode);
	}
	void ItemSamples::Load(const KxXMLNode& samplesNode)
	{
		if (samplesNode.IsOK())
		{
			m_SourceType.FromString(samplesNode.GetAttribute(wxS("Source")));
			switch (m_SourceType.GetValue())
			{
				case SamplesSourceID::ImmediateItems:
				{
					m_SortOrder.FromString(samplesNode.GetAttribute(wxS("SortOrder")));
					m_SortOptions.FromOrExpression(samplesNode.GetAttribute(wxS("SortOptions")));

					m_Values.reserve(samplesNode.GetChildrenCount());
					LoadImmediateItems(samplesNode);
					if (!m_SortOrder.IsDefault())
					{
						SortImmediateItems();
					}
					break;
				}
				case SamplesSourceID::Range:
				{
					const TypeID type = m_Item.GetTypeID();
					bool isRangeLoaded = false;

					if (type.IsFloat())
					{
						double min = samplesNode.GetAttributeFloat(wxS("Min"), 0);
						double max = samplesNode.GetAttributeFloat(wxS("Max"), 0);
						double step = samplesNode.GetAttributeFloat(wxS("Step"), 1.0);
						m_SortOrder = LoadRange(min, max, step);
						isRangeLoaded = true;
					}
					else if (type.IsSignedInteger())
					{
						int64_t min = samplesNode.GetAttributeInt(wxS("Min"), 0);
						int64_t max = samplesNode.GetAttributeInt(wxS("Max"), 0);
						int64_t step = samplesNode.GetAttributeInt(wxS("Step"), 1);
						m_SortOrder = LoadRange(min, max, step);
						isRangeLoaded = true;
					}
					else if (type.IsUnsignedInteger())
					{
						uint64_t min = samplesNode.GetAttributeInt(wxS("Min"), 0);
						uint64_t max = samplesNode.GetAttributeInt(wxS("Max"), 0);
						uint64_t step = samplesNode.GetAttributeInt(wxS("Step"), 1);
						m_SortOrder = LoadRange(min, max, step);
						isRangeLoaded = true;
					}

					if (isRangeLoaded && LoadImmediateItems(samplesNode) != 0)
					{
						SortImmediateItems();
					}
					break;
				}
				case SamplesSourceID::Function:
				{
					const KxXMLNode functionNode = samplesNode.GetFirstChildElement(wxS("Function"));
					if (m_SampligFunction.FromString(functionNode.GetAttribute(wxS("Name"))))
					{
						ItemValue::Vector arguments;

						for (KxXMLNode argNode = functionNode.GetFirstChildElement(wxS("Arg")); argNode.IsOK(); argNode = argNode.GetNextSiblingElement(wxS("Arg")))
						{
							TypeID type;
							if (type.FromString(argNode.GetAttribute(wxS("Type"))))
							{
								SimpleItem item(m_Item.GetGroup());
								item.SetTypeID(type);
								item.GetValue().Deserialize(argNode.GetValue(), item);

								arguments.emplace_back(type);
							}
						}
						GenerateItems(arguments);
					}
					break;
				}
			}
		}
	}
}
