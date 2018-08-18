#pragma once
#include "stdafx.h"

class KCMConfigEntryStd;
class KCMSampleValue
{
	public:
		static bool SortComparator(const KCMSampleValue& v1, const KCMSampleValue& v2);

	private:
		wxString m_Value;
		wxString m_Label;

	public:
		KCMSampleValue(const KCMConfigEntryStd* configEntry, const wxString& value, const wxString& label = wxEmptyString);
		KCMSampleValue(const KCMConfigEntryStd* configEntry, KxXMLNode& node);

	public:
		const wxString& GetValue() const
		{
			return m_Value;
		}
		const wxString& GetLabel() const
		{
			return m_Label;
		}

	public:
		bool operator<(const KCMSampleValue& other) const
		{
			return m_Value < other.m_Value;
		}
};
typedef std::vector<KCMSampleValue> KCMSampleValueArray;
