#pragma once
#include "stdafx.h"

class KLabeledValue: public wxClientDataContainer
{
	private:
		wxString m_Value;
		wxString m_Label;

	public:
		KLabeledValue(const wxString& value, const wxString& label = wxEmptyString)
			:m_Value(value), m_Label(label)
		{
		}
		virtual ~KLabeledValue()
		{
		}

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
		const wxString& GetLabelRaw() const
		{
			return m_Label;
		}
		const wxString& GetLabel() const
		{
			return HasLabel() ? GetLabelRaw() : GetValue();
		}
		void SetLabel(const wxString& label)
		{
			m_Label = label;
		}
};
typedef std::vector<KLabeledValue> KLabeledValueArray;
