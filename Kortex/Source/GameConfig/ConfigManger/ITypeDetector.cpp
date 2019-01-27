#include "stdafx.h"
#include "ITypeDetector.h"

namespace Kortex::GameConfig
{
	HungarianNotationTypeDetector::HungarianNotationTypeDetector(const KxXMLNode& rootNode)
	{
		for (KxXMLNode node = rootNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{
			auto[it, inserted] = m_TypeMap.insert_or_assign(node.GetAttribute(wxS("Prefix")), TypeID(node.GetAttribute(wxS("Type"))));
			if (it->second.IsNone())
			{
				m_TypeMap.erase(it);
			}
		}
	}

	TypeID HungarianNotationTypeDetector::GetType(const wxString& valueName, const wxString& valueData) const
	{
		if (!valueName.IsEmpty())
		{
			for (const auto&[prefix, typeID]: m_TypeMap)
			{
				if (valueName.StartsWith(prefix))
				{
					return typeID;
				}
			}
		}
		return {};
	}
}

namespace Kortex::GameConfig
{
	TypeID DataAnalysisTypeDetector::GetType(const wxString& valueName, const wxString& valueData) const
	{
		if (!valueData.IsEmpty())
		{
			// Test for bool. Exactly these four variants, no mixed case.
			if (valueData == wxS("true") || valueData == wxS("TRUE") || valueData == wxS("false") || valueData == wxS("FALSE"))
			{
				return DataTypeID::Bool;
			}

			// Test for floats
			double value = 0;
			if (valueData.find(wxS('.')) != wxString::npos && valueData.ToCDouble(&value))
			{
				if (value >= (double)std::numeric_limits<float>::max() || value <= (double)std::numeric_limits<float>::lowest())
				{
					return DataTypeID::Float64;
				}
				else
				{
					return DataTypeID::Float32;
				}
			}
			
			// Test for int. Not really reliable though.
			if (long iValue = 0; valueData.ToCLong(&iValue))
			{
				return DataTypeID::Int32;
			}
			else if (unsigned long uValue = 0; valueData.ToCULong(&uValue))
			{
				return DataTypeID::UInt32;
			}
			else if (long long llValue = 0; valueData.ToLongLong(&llValue))
			{
				return DataTypeID::Int64;
			}
			else if (unsigned long long ullValue = 0; valueData.ToULongLong(&ullValue))
			{
				return DataTypeID::UInt64;
			}

			// Nothing we can do, let it be a string
			return DataTypeID::String;
		}
		return {};
	}
}
