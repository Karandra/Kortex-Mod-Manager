#include "stdafx.h"
#include "KCMValueTypeDetector.h"
#include "KConfigManager.h"

void KCMHungarianNotationDetector::Init(KxXMLNode& node)
{
	for (node = node.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
	{
		wxString sPrefix = node.GetAttribute("Prefix");
		if (!sPrefix.IsEmpty())
		{
			wxString sTypeName = node.GetValue();
			m_TypesMap.emplace(std::make_pair(sPrefix, KConfigManager::GetTypeID(sTypeName)));
		}
	}
}
KCMDataType KCMHungarianNotationDetector::operator()(const wxString& valueName, const wxString& valueData)
{
	if (!valueName.IsEmpty())
	{
		for (const auto& v: m_TypesMap)
		{
			if (valueName.StartsWith(v.first))
			{
				return v.second;
			}
		}
	}
	return KCMDT_UNKNOWN;
}

KCMDataType KCMDataAnalysisDetector::operator()(const wxString& valueName, const wxString& valueData)
{
	if (!valueData.IsEmpty())
	{
		double dValue = 0;
		long iValue = 0;
		unsigned long uValue = 0;
		long long llValue = 0;
		unsigned long long ullValue = 0;

		if (valueData.IsSameAs("true", false) || valueData.IsSameAs("false", false))
		{
			return KCMDT_BOOL;
		}
		else if (valueData.Find('.') != wxNOT_FOUND && valueData.ToCDouble(&dValue))
		{
			// Can't really distinguish float32 from float64 here so use float32
			return KCMDT_FLOAT32;
		}
		else if (valueData.ToCLong(&iValue))
		{
			return KCMDT_INT32;
		}
		else if (valueData.ToCULong(&uValue))
		{
			return KCMDT_UINT32;
		}
		else if (valueData.ToLongLong(&llValue))
		{
			return KCMDT_INT64;
		}
		else if (valueData.ToULongLong(&ullValue))
		{
			return KCMDT_UINT64;
		}
		else
		{
			return KCMDT_STRING;
		}
	}
	return KCMDT_UNKNOWN;
}
