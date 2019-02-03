#pragma once
#include "stdafx.h"
#include "Common.h"
#include "ItemValue.h"
class KxXMLNode;

namespace Kortex::GameConfig
{
	class Item;

	enum class CompareResult
	{
		LessThan = 1,
		Equal = 2,
		GreaterThan = 3
	};
	CompareResult CompareStrings(const wxString& v1, const wxString& v2, SortOptionsValue sortOptions);
}

namespace Kortex::GameConfig
{
	enum class SamplingFunctionID: uint32_t
	{
		None = 0,
		GetVideoModes,
		GetVideoAdapters,
		GetVirtualKeys,
		FindFiles,
	};
	class SamplingFunctionDef: public KxIndexedEnum::Definition<SamplingFunctionDef, SamplingFunctionID, wxString, true>
	{
		inline static const TItem ms_Index[] =
		{
			{SamplingFunctionID::None, wxS("None")},
			{SamplingFunctionID::GetVideoModes, wxS("GetVideoModes")},
			{SamplingFunctionID::GetVideoAdapters, wxS("GetVideoAdapters")},
			{SamplingFunctionID::GetVirtualKeys, wxS("GetVirtualKeys")},
			{SamplingFunctionID::FindFiles, wxS("FindFiles")},
		};
	};
	using SamplingFunctionValue = KxIndexedEnum::Value<SamplingFunctionDef, SamplingFunctionID::None>;
}

namespace Kortex::GameConfig
{
	class SampleValue
	{
		private:
			ItemValue m_Value;
			wxString m_Label;

		public:
			SampleValue(const wxString& label = {})
				:m_Label(label)
			{
			}
			template<class T> SampleValue(const wxString& label, T&& value)
				:m_Label(label), m_Value(std::forward<T>(value))
			{
			}

		public:
			const ItemValue& GetValue() const
			{
				return m_Value;
			}
			ItemValue& GetValue()
			{
				return m_Value;
			}

			bool HasLabel() const
			{
				return !m_Label.IsEmpty();
			}
			wxString GetLabel() const
			{
				return m_Label.IsEmpty() ? m_Value.As<wxString>() : m_Label;
			}
			void SetLabel(const wxString& label)
			{
				m_Label = label;
			}
	};
}

namespace Kortex::GameConfig
{
	class ItemSamples
	{
		private:
			struct VirtualKeyInfo
			{
				using Map = std::unordered_map<uint64_t, VirtualKeyInfo>;

				wxString ID;
				wxString Name;
				uint64_t Code = 0;
			};

		private:
			Item& m_Item;
			SamplesSourceValue m_SourceType;
			SortOrderValue m_SortOrder;
			SortOptionsValue m_SortOptions;
			SamplingFunctionValue m_SampligFunction;
			std::vector<SampleValue> m_Values;

		private:
			size_t LoadImmediateItems(const KxXMLNode& rootNode);
			void SortImmediateItems();
			void GenerateItems(const ItemValue::Vector& arguments);
			const VirtualKeyInfo::Map& LoadVirtualKeys();
			template<class T> SortOrderValue LoadRange(T min, T max, T step)
			{
				if constexpr(std::is_signed_v<T>)
				{
					m_Values.reserve(std::abs(max - min));
				}

				for (T i = min; i <= max; i += step)
				{
					SampleValue& sample = m_Values.emplace_back();
					sample.GetValue().Assign(i);
				}
				return step >= 0 ? SortOrderID::Ascending : SortOrderID::Descending;
			}

			template<class TItems, class TFunctor> void DoForEachItem(TItems&& items, TFunctor&& func)
			{
				for (auto& item: items)
				{
					func(item);
				}
			}

		public:
			ItemSamples(Item& item, const KxXMLNode& samplesNode = {});

		public:
			Item& GetItem() const
			{
				return m_Item;
			}
			void Load(const KxXMLNode& samplesNode);

			SamplesSourceValue GetSourceType() const
			{
				return m_SourceType;
			}
			SortOrderValue GetSortOrder() const
			{
				return m_SortOrder;
			}
			SortOptionsValue GetSortOptions() const
			{
				return m_SortOptions;
			}
			SamplingFunctionValue GetSamplingFunction() const
			{
				return m_SampligFunction;
			}

			template<class TFunctor> void ForEachSample(TFunctor&& func) const
			{
				DoForEachItem(m_Values, func);
			}
			template<class TFunctor> void ForEachSample(TFunctor&& func)
			{
				DoForEachItem(m_Values, func);
			}
	
			const SampleValue* FindSampleByValue(const ItemValue& value) const;
			const SampleValue* FindSampleByLabel(const wxString& label) const;
	};
}
