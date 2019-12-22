#pragma once
#include "stdafx.h"

// TODO: Revise usage of this class. Remove it if needed
namespace Kortex::Utility
{
	class LabeledValue: public wxClientDataContainer
	{
		public:
			using Vector = std::vector<LabeledValue>;
			using RefVector = std::vector<LabeledValue*>;

		private:
			wxString m_Value;
			wxString m_Label;

		public:
			LabeledValue(const wxString& value, const wxString& label = wxEmptyString)
				:m_Value(value), m_Label(label)
			{
			}
			virtual ~LabeledValue() = default;

		public:
			bool HasValue() const
			{
				return !m_Value.IsEmpty();
			}
			const wxString& GetValue() const
			{
				return m_Value;
			}
			void SetValue(const wxString& value)
			{
				m_Value = value;
			}
		
			bool HasLabel() const
			{
				return !m_Label.IsEmpty();
			}
			const wxString& GetRawLabel() const
			{
				return m_Label;
			}
			const wxString& GetLabel() const
			{
				return HasLabel() ? GetRawLabel() : GetValue();
			}
			void SetLabel(const wxString& label)
			{
				m_Label = label;
			}
	};
}

namespace Kortex::Utility
{
	inline LabeledValue::Vector ExpandVariablesInVector(const LabeledValue::Vector& items)
	{
		LabeledValue::Vector newItems;
		newItems.reserve(items.size());
		for (const LabeledValue& item: items)
		{
			newItems.emplace_back(KVarExp(item.GetValue()), KVarExp(item.GetRawLabel()));
		}
		return newItems;
	}
}
