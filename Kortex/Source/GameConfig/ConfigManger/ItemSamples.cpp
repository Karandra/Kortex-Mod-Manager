#include "stdafx.h"
#include "ItemSamples.h"
#include "Item.h"
#include "Items/SimpleItem.h"
#include "SamplingFunctions/GetVideoAdapters.h"
#include "SamplingFunctions/GetVideoModes.h"
#include "SamplingFunctions/GetVirtualKeys.h"
#include "SamplingFunctions/GetAvailableTranslations.h"
#include "SamplingFunctions/GetStartupWorkspaces.h"
#include "SamplingFunctions/FindFiles.h"
#include "GameConfig/IConfigManager.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxSystem.h>

namespace
{
	class CompareStringsData
	{
		private:
			NLSVERSIONINFOEX m_VersionInfo = {0};
			bool m_OK = false;

		public:
			CompareStringsData(const wchar_t* localeName)
			{
				m_VersionInfo.dwNLSVersionInfoSize = sizeof(m_VersionInfo);

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
}

namespace Kortex::GameConfig
{
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
		m_Values.reserve(rootNode.GetChildrenCount());

		for (KxXMLNode node = rootNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{
			SampleValue& sample = m_Values.emplace_back();
			sample.SetLabel(m_Item.GetManager().TranslateItemLabel(node, {}, wxS("SampleValue"), true));
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
			case SamplingFunctionID::FindFiles:
			{
				SamplingFunction::FindFiles(m_Values).Invoke(arguments);
				break;
			}
			case SamplingFunctionID::GetAvailableTranslations:
			{
				SamplingFunction::GetAvailableTranslations(m_Values).Invoke(arguments);
				break;
			}
			case SamplingFunctionID::GetStartupWorkspaces:
			{
				SamplingFunction::GetStartupWorkspaces(m_Values).Invoke(arguments);
				break;
			}
			case SamplingFunctionID::GetVideoAdapters:
			{
				SamplingFunction::GetVideoAdapters(m_Values).Invoke(arguments);
				break;
			}
			case SamplingFunctionID::GetVideoModes:
			{
				SamplingFunction::GetVideoModes(m_Values, m_Item.GetManager()).Invoke(arguments);
				break;
			}
			case SamplingFunctionID::GetVirtualKeys:
			{
				SamplingFunction::GetVirtualKeys(m_Values, m_Item.GetManager()).Invoke(arguments);

				m_SortOrder = SortOrderID::Ascending;
				m_SortOptions.AddFlag(SortOptionsID::DigitsAsNumbers);
				m_SortOptions.AddFlag(SortOptionsID::IgnoreCase);
				SortImmediateItems();
				break;
			}
		};
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
						double max = samplesNode.GetAttributeFloat(wxS("Max"), 1.0);
						double step = samplesNode.GetAttributeFloat(wxS("Step"), 1.0);
						m_SortOrder = LoadRange(min, max, step);
						isRangeLoaded = true;
					}
					else if (type.IsSignedInteger())
					{
						int64_t min = samplesNode.GetAttributeInt(wxS("Min"), 0);
						int64_t max = samplesNode.GetAttributeInt(wxS("Max"), 1);
						int64_t step = samplesNode.GetAttributeInt(wxS("Step"), 1);
						m_SortOrder = LoadRange(min, max, step);
						isRangeLoaded = true;
					}
					else if (type.IsUnsignedInteger())
					{
						uint64_t min = samplesNode.GetAttributeInt(wxS("Min"), 0);
						uint64_t max = samplesNode.GetAttributeInt(wxS("Max"), 1);
						uint64_t step = samplesNode.GetAttributeInt(wxS("Step"), 1);
						m_SortOrder = LoadRange(min, max, step);
						isRangeLoaded = true;
					}

					if (isRangeLoaded && LoadImmediateItems(samplesNode) != 0)
					{
						m_SortOptions.AddFlag(SortOptionsID::DigitsAsNumbers);
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

								arguments.emplace_back(std::move(item.GetValue()));
							}
						}
						GenerateItems(arguments);
					}
					break;
				}
			}
		}
	}

	bool ItemSamples::HasStep() const
	{
		return !m_Step.IsNull();
	}
	bool ItemSamples::HasBoundValues() const
	{
		return !m_MinValue.IsNull() && !m_MaxValue.IsNull();
	}
	
	const SampleValue* ItemSamples::FindSampleByValue(const ItemValue& value, size_t* index) const
	{
		if (!value.IsNull())
		{
			size_t counter = 0;
			for (const SampleValue& sampleValue: m_Values)
			{
				if (sampleValue.GetValue().As<wxString>() == value.As<wxString>())
				{
					KxUtility::SetIfNotNull(index, counter);
					return &sampleValue;
				}
				counter++;
			}
		}
		return nullptr;
	}
	const SampleValue* ItemSamples::FindSampleByLabel(const wxString& label, size_t* index) const
	{
		size_t counter = 0;
		for (const SampleValue& sampleValue: m_Values)
		{
			if (sampleValue.GetLabel() == label)
			{
				KxUtility::SetIfNotNull(index, counter);
				return &sampleValue;
			}
			counter++;
		}
		return nullptr;
	}
}
