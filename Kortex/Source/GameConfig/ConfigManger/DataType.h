#pragma once
#include "stdafx.h"
#include "Common.h"
#include <KxFramework/KxIndexedEnum.h>
class KxXMLNode;

namespace Kortex::GameConfig
{
	class TypeID: public DataTypeValue
	{
		public:
			struct Hash
			{
				size_t operator()(TypeID id) const noexcept
				{
					return std::hash<TypeID::TInt>()(id.GetInt());
				}
			};

		public:
			template<class T> static TypeID GetByCType()
			{
				if constexpr(std::is_same_v<T, int8_t>)
				{
					return DataTypeID::Int8;
				}
				if constexpr(std::is_same_v<T, int16_t>)
				{
					return DataTypeID::Int16;
				}
				if constexpr (std::is_same_v<T, int32_t>)
				{
					return DataTypeID::Int32;
				}
				if constexpr (std::is_same_v<T, int64_t>)
				{
					return DataTypeID::Int64;
				}
				if constexpr(std::is_same_v<T, uint8_t>)
				{
					return DataTypeID::UInt8;
				}
				if constexpr(std::is_same_v<T, uint16_t>)
				{
					return DataTypeID::UInt16;
				}
				if constexpr (std::is_same_v<T, uint32_t>)
				{
					return DataTypeID::UInt32;
				}
				if constexpr (std::is_same_v<T, uint64_t>)
				{
					return DataTypeID::UInt64;
				}

				if constexpr(std::is_same_v<T, float>)
				{
					return DataTypeID::Float32;
				}
				if constexpr(std::is_same_v<T, double>)
				{
					return DataTypeID::Float64;
				}

				if constexpr(std::is_same_v<T, bool>)
				{
					return DataTypeID::Bool;
				}
				if constexpr(std::is_same_v<T, wxString> || std::is_same_v<T, const char*> || std::is_same_v<T, const wchar_t*>)
				{
					return DataTypeID::String;
				}
				return DataTypeID::None;
			}

		private:
			template<DataTypeID... id> bool IsOneOfTypes() const
			{
				return (HasFlag(id) || ...);
			}

		public:
			TypeID() = default;
			TypeID(TEnum value)
				:DataTypeValue(value)
			{
			}
			TypeID(TInt value)
				:DataTypeValue(value)
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
			bool IsDefinitiveType() const
			{
				return !IsNone() && !IsAny();
			}
			
			bool IsSignedInteger() const;
			bool IsUnsignedInteger() const;
			bool IsAnyInt64() const;
			bool IsInteger() const;
			bool IsFloat() const;
			bool IsBool() const;
			bool IsString() const;
			bool IsStruct() const;
			bool IsType(DataTypeID id) const
			{
				return HasFlag(id);
			}
	};
}

namespace Kortex::GameConfig
{
	class DataType
	{
		public:
			static DataType CreateGeneric(DataTypeID id, int precision = -1);

		public:
			TypeID m_TypeID;
			TypeID m_InputType;
			TypeID m_OutputType;
			int m_Precision = -1;

		public:
			DataType() = default;
			DataType(const KxXMLNode& node);

		public:
			bool IsOK() const;

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
