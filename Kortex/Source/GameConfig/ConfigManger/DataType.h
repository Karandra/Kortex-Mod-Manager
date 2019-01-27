#pragma once
#include "stdafx.h"
#include "Common.h"
#include <KxFramework/KxIndexedEnum.h>
class KxXMLNode;

namespace Kortex::GameConfig
{
	class TypeID: public DataTypeIDValue
	{
		private:
			template<DataTypeID... IDs> bool IsOneOfTypes() const
			{
				return ((GetValue() == IDs) || ...);
			}

		public:
			TypeID() = default;
			TypeID(TEnum value)
				:DataTypeIDValue(value)
			{
			}
			TypeID(TInt value)
				:DataTypeIDValue(value)
			{
			}
			TypeID(const wxString& value)
			{
				FromString(value);
			}
			
		public:
			bool IsNone() const
			{
				return IsDefault();
			}
			bool IsAny() const
			{
				return GetValue() == DataTypeID::Any;
			}
			bool IsInteger() const;
			bool IsFloat() const;
			bool IsBool() const;
			bool IsString() const;
	};
}

namespace Kortex::GameConfig
{
	class DataType
	{
		public:
			DataType CreateGeneric(DataTypeID id, int precision = -1);

		public:
			TypeID m_TypeID;
			TypeID m_InputType;
			TypeID m_OutputType;
			int m_Precision = -1;

		public:
			DataType() = default;
			DataType(const KxXMLNode& node);

		public:
			bool IsOK() const
			{
				return m_TypeID != DataTypeID::None;
			}

			TypeID GetID() const
			{
				return m_TypeID;
			}
			TypeID GetInputType() const
			{
				return m_InputType;
			}
			TypeID GetOutputType() const
			{
				return m_OutputType;
			}
			int GetPrecision() const
			{
				return m_Precision;
			}
	};
}
