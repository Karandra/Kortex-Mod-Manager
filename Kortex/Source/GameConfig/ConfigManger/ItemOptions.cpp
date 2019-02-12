#include "stdafx.h"
#include "ItemOptions.h"
#include <KxFramework/KxXML.h>

namespace
{
	using IOC = Kortex::GameConfig::ItemOptionsCopy;

	template<class T> void CopyIfDefault(T& thisValue, const T& otherValue, IOC copyWhat, IOC thisType)
	{
		if (copyWhat & thisType && thisValue.IsDefault())
		{
			thisValue = otherValue;
		}
	}
	template<> void CopyIfDefault(wxString& thisValue, const wxString& otherValue, IOC copyWhat, IOC thisType)
	{
		if (copyWhat & thisType && thisValue.IsEmpty())
		{
			thisValue = otherValue;
		}
	}
	template<> void CopyIfDefault(int& thisValue, const int& otherValue, IOC copyWhat, IOC thisType)
	{
		if (copyWhat & thisType && thisValue < 0)
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
			m_EditableBehavior.FromString(node.GetFirstChildElement(wxS("EditableBehavior")).GetAttribute(wxS("Value")));

			// Load precision
			if (dataType.IsOK() && dataType.GetOutputType().IsFloat())
			{
				m_Precision = node.GetAttributeInt(wxS("Precision"), -1);
			}
			else
			{
				m_Precision = node.GetFirstChildElement(wxS("Output")).GetAttributeInt(wxS("Precision"), -1);
			}

			int maxDigits = 0;
			if (dataType.GetID() == DataTypeID::Float32)
			{
				maxDigits = std::numeric_limits<float>::digits10;
			}
			else if (dataType.GetID() == DataTypeID::Float64)
			{
				maxDigits = std::numeric_limits<double>::digits10;
			}
			m_Precision = std::clamp(m_Precision, -1, maxDigits);
		}
	}
	void ItemOptions::CopyIfNotSpecified(const ItemOptions& other, const DataType& dataType, ItemOptionsCopy copyWhat)
	{
		CopyIfDefault(m_InputFormat, other.m_InputFormat, copyWhat, ItemOptionsCopy::InputFormat);
		CopyIfDefault(m_OutputFormat, other.m_OutputFormat, copyWhat, ItemOptionsCopy::OutputFormat);

		CopyIfDefault(m_SourceFormat, other.m_SourceFormat, copyWhat, ItemOptionsCopy::SourceFormat);
		CopyIfDefault(m_TypeDetector, other.m_TypeDetector, copyWhat, ItemOptionsCopy::TypeDetector);
		CopyIfDefault(m_EditableBehavior, other.m_EditableBehavior, copyWhat, ItemOptionsCopy::EditableBehavior);

		CopyIfDefault(m_Precision, other.m_Precision, copyWhat, ItemOptionsCopy::Precision);
		CopyIfDefault(m_Precision, dataType.GetPrecision(), copyWhat, ItemOptionsCopy::Precision);
	}
}
