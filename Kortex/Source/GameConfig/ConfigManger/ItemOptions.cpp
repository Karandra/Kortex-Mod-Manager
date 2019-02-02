#include "stdafx.h"
#include "ItemOptions.h"
#include <KxFramework/KxXML.h>

namespace
{
	template<class T> void CopyIfDefault(T& thisValue, const T& otherValue)
	{
		if (thisValue.IsDefault())
		{
			thisValue = otherValue;
		}
	}
	template<> void CopyIfDefault(wxString& thisValue, const wxString& otherValue)
	{
		if (thisValue.IsEmpty())
		{
			thisValue = otherValue;
		}
	}
	template<> void CopyIfDefault(int& thisValue, const int& otherValue)
	{
		if (thisValue < 0)
		{
			thisValue = otherValue;
		}
	}
}

namespace Kortex::GameConfig
{
	void ItemOptions::Load(const KxXMLNode& node, const DataType& dataType)
	{
		if (node.IsOK())
		{
			m_InputFormat = node.GetFirstChildElement(wxS("Input")).GetFirstChildElement(wxS("Format")).GetValue();
			m_OutputFormat = node.GetFirstChildElement(wxS("Output")).GetFirstChildElement(wxS("Format")).GetValue();

			m_SourceFormat.FromString(node.GetFirstChildElement(wxS("SourceFormat")).GetAttribute(wxS("Value")));
			m_TypeDetector.FromString(node.GetFirstChildElement(wxS("TypeDetection")).GetAttribute(wxS("Value")));

			// Load precision
			if (dataType.IsOK() && dataType.GetOutputType().IsFloat())
			{
				m_Precision = node.GetAttributeInt(wxS("Precision"), -1);
			}
			else
			{
				m_Precision = node.GetFirstChildElement(wxS("Output")).GetAttributeInt(wxS("Precision"), -1);
			}

			// I think 8 digits of precision should be enough
			m_Precision = std::clamp(m_Precision, -1, 8);
		}
	}
	void ItemOptions::CopyIfNotSpecified(const ItemOptions& other)
	{
		CopyIfDefault(m_InputFormat, other.m_InputFormat);
		CopyIfDefault(m_OutputFormat, other.m_OutputFormat);

		CopyIfDefault(m_SourceFormat, other.m_SourceFormat);
		CopyIfDefault(m_TypeDetector, other.m_TypeDetector);

		CopyIfDefault(m_Precision, other.m_Precision);
	}
}