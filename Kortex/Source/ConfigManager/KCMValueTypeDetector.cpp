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
KCMDataType KCMHungarianNotationDetector::operator()(const wxString& sValueName, const wxString& sValueData)
{
	if (!sValueName.IsEmpty())
	{
		for (const auto& v: m_TypesMap)
		{
			if (sValueName.StartsWith(v.first))
			{
				return v.second;
			}
		}
	}
	return KCMDT_UNKNOWN;
}

KCMDataType KCMDataAnalysisDetector::operator()(const wxString& sValueName, const wxString& sValueData)
{
	if (!sValueData.IsEmpty())
	{
		double dValue = 0;
		long iValue = 0;
		unsigned long uValue = 0;
		long long llValue = 0;
		unsigned long long ullValue = 0;

		if (sValueData.IsSameAs("true", false) || sValueData.IsSameAs("false", false))
		{
			return KCMDT_BOOL;
		}
		else if (sValueData.Find('.') != wxNOT_FOUND && sValueData.ToCDouble(&dValue))
		{
			// Can't really distinguish float32 from float64 here so use float32
			return KCMDT_FLOAT32;
		}
		else if (sValueData.ToCLong(&iValue))
		{
			return KCMDT_INT32;
		}
		else if (sValueData.ToCULong(&uValue))
		{
			return KCMDT_UINT32;
		}
		else if (sValueData.ToLongLong(&llValue))
		{
			return KCMDT_INT64;
		}
		else if (sValueData.ToULongLong(&ullValue))
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
