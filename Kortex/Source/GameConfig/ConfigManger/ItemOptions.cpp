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
}

namespace Kortex::GameConfig
{
	void ItemOptions::Load(const KxXMLNode& node)
	{
		if (node.IsOK())
		{
			m_SourceFormat.FromString(node.GetFirstChildElement(wxS("Format")).GetAttribute(wxS("Value")));
			m_TypeDetector.FromString(node.GetFirstChildElement(wxS("TypeDetection")).GetAttribute(wxS("Value")));
		}
	}
	void ItemOptions::CopyIfNotSpecified(const ItemOptions& other)
	{
		CopyIfDefault(m_SourceFormat, other.m_SourceFormat);
		CopyIfDefault(m_TypeDetector, other.m_TypeDetector);
	}
}
