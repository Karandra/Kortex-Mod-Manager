#include "stdafx.h"
#include "DataType.h"
#include <KxFramework/KxXML.h>

namespace Kortex::GameConfig
{
	bool TypeID::IsSignedInteger() const
	{
		return IsOneOfTypes<DataTypeID::Int8, DataTypeID::Int16, DataTypeID::Int32>();
	}
	bool TypeID::IsUnsignedInteger() const
	{
		return IsOneOfTypes<DataTypeID::UInt8, DataTypeID::UInt16, DataTypeID::UInt32, DataTypeID::UInt64>();
	}
	bool TypeID::IsInteger() const
	{
		return IsSignedInteger() || IsUnsignedInteger();
	}
	bool TypeID::IsFloat() const
	{
		return IsOneOfTypes<DataTypeID::Float32, DataTypeID::Float64>();
	}
	bool TypeID::IsBool() const
	{
		return IsOneOfTypes<DataTypeID::Bool>();
	}
	bool TypeID::IsString() const
	{
		return IsOneOfTypes<DataTypeID::String>();
	}
}

namespace Kortex::GameConfig
{
	DataType DataType::CreateGeneric(DataTypeID id, int precision)
	{
		DataType type;
		type.m_TypeID = id;
		type.m_InputType = DataTypeID::Any;
		type.m_OutputType = id;

		if (precision >= 0 && type.m_TypeID.IsFloat())
		{
			type.m_Precision = precision;
		}
		return type;
	}

	DataType::DataType(const KxXMLNode& node)
	{
		if (node.IsOK() && m_TypeID.FromString(node.GetAttribute(wxS("Type"))) && !m_TypeID.IsAny())
		{
			// Input
			const KxXMLNode inputNode = node.GetFirstChildElement(wxS("Input"));
			m_InputType.FromOrExpression(inputNode.GetAttribute(wxS("As")), DataTypeID::Any);
			
			// Output
			const KxXMLNode outputNode = node.GetFirstChildElement(wxS("Output"));

			m_OutputType.FromOrExpression(outputNode.GetAttribute(wxS("As")), m_TypeID);
			if (m_OutputType.IsFloat())
			{
				// I think 8 digits of precision should be enough
				m_Precision = std::clamp<int>(outputNode.GetAttributeInt(wxS("Precision"), -1), -1, 8);
			}
		}
	}

	bool DataType::IsOK() const
	{
		return !m_TypeID.IsNone() && !m_TypeID.IsAny() && !m_InputType.IsNone() && !m_OutputType.IsNone() && !m_OutputType.IsAny();
	}
}
