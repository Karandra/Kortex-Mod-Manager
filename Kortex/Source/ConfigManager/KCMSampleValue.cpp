#include "stdafx.h"
#include "KCMSampleValue.h"
#include "KCMConfigEntry.h"
#include "KApp.h"
#include <KxFramework/KxXML.h>

bool KCMSampleValue::SortComparator(const KCMSampleValue& v1, const KCMSampleValue& v2)
{
	// Compare as double, should be enough.
	auto nValue1 = std::wcstold(v1.GetValue(), NULL);
	auto nValue2 = std::wcstold(v2.GetValue(), NULL);
	return nValue1 < nValue2;
}

KCMSampleValue::KCMSampleValue(const KCMConfigEntryStd* configEntry, const wxString& value, const wxString& label)
	:m_Value(value), m_Label(label)
{
}
KCMSampleValue::KCMSampleValue(const KCMConfigEntryStd* configEntry, KxXMLNode& node)
{
	m_Value = node.GetValue();
	if (node.HasAttribute("Label"))
	{
		m_Label = KApp::Get().ExpandVariables(node.GetAttribute("Label"));
	}
}
