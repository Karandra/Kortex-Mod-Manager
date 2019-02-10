#include "stdafx.h"
#include "ItemValue.h"
#include "ItemOptions.h"
#include "Item.h"
#include <KxFramework/KxString.h>

namespace
{
	using Kortex::GameConfig::Item;
	using Kortex::GameConfig::ItemOptions;

	template<class T> wxString GetAs(const wxAny& anyValue, const Item& item)
	{
		T value;
		if (anyValue.GetAs(&value))
		{
			return value;
		}
		return {};
	}
	template<class T> wxString FormatAs(const T& value, const Item& item)
	{
		const ItemOptions& options = item.GetOptions();

		KxFormat formatter(options.GetOutputFormat());
		if constexpr(std::is_floating_point_v<T>)
		{
			return formatter(value, options.GetPrecision());
		}
		else
		{
			return formatter(value);
		}
	}
	template<class T> wxString GetAndFormat(const wxAny& anyValue, const Item& item)
	{
		T value;
		if (anyValue.GetAs(&value))
		{
			return FormatAs(value, item);
		}
		return {};
	}

	namespace ToAny
	{
		bool GetAsBool(bool& value, const wxString& stringValue)
		{
			if (stringValue == wxS("true") || stringValue == wxS("TRUE") || stringValue == wxS("1"))
			{
				value = true;
				return true;
			}
			else if (stringValue == wxS("false") || stringValue == wxS("FALSE") || stringValue == wxS("0"))
			{
				value = false;
				return true;
			}
			return false;
		}
		template<class T> bool GetAsSignedInteger(T& value, const wxString& stringValue)
		{
			static_assert(std::is_signed_v<T>);

			long long iValue = 0;
			if (stringValue.ToLongLong(&iValue))
			{
				value = iValue;
				return true;
			}
			return false;
		}
		template<class T> bool GetAsUnsignedInteger(T& value, const wxString& stringValue)
		{
			static_assert(std::is_unsigned_v<T>);

			unsigned long long iValue = 0;
			if (stringValue.ToULongLong(&iValue))
			{
				value = iValue;
				return true;
			}
			return false;
		}
		template<class T> bool GetAsFloat(T& value, const wxString& stringValue)
		{
			static_assert(std::is_floating_point_v<T>);

			double dValue = 0;
			if (stringValue.ToCDouble(&dValue))
			{
				value = dValue;
				return true;
			}
			return false;
		}
	}

	namespace FromAny
	{
		wxString AsBool(const wxAny& value, const Item& item)
		{
			bool boolValue = false;
			value.GetAs(&boolValue);

			return boolValue ? wxS("true") : wxS("false");
		}
		wxString AsSignedInteger(const wxAny& value, const Item& item)
		{
			return GetAndFormat<int64_t>(value, item);
		}
		wxString AsUnsignedInteger(const wxAny& value, const Item& item)
		{
			return GetAndFormat<uint64_t>(value, item);
		}
		wxString AsFloat32(const wxAny& value, const Item& item)
		{
			return GetAndFormat<float>(value, item);
		}
		wxString AsFloat64(const wxAny& value, const Item& item)
		{
			return GetAndFormat<double>(value, item);
		}
		wxString AsString(const wxAny& value, const Item& item)
		{
			return GetAndFormat<wxString>(value, item);
		}
	}
}

namespace Kortex::GameConfig
{
	void ItemValue::DoDeserialize(const wxString& stringValue, const Item& item)
	{
		const TypeID type = item.GetTypeID();
		const TypeID inputType = item.GetDataType().GetInputType();

		if (type.IsBool())
		{
			DeserializeAsBool(inputType, stringValue);
		}
		else if (type.IsSignedInteger())
		{
			DeserializeAsSignedInteger(inputType, stringValue);
		}
		else if (type.IsUnsignedInteger())
		{
			DeserializeAsUnsignedInteger(inputType, stringValue);
		}
		else if (type.IsFloat())
		{
			DeserializeAsFloat(inputType, stringValue);
		}
		else if (type.IsString())
		{
			DeserializeAsString(inputType, stringValue);
		}
	}
	void ItemValue::DeserializeAsBool(TypeID inputType, const wxString& stringValue)
	{
		if (inputType.IsBool())
		{
			bool value = false;
			if (ToAny::GetAsBool(value, stringValue))
			{
				m_Type = DataTypeID::Bool;
				m_Value = value;
				return;
			}
		}
		if (inputType.IsSignedInteger())
		{
			int value = 0;
			if (ToAny::GetAsSignedInteger(value, stringValue))
			{
				m_Type = DataTypeID::Bool;
				m_Value = value != 0;
				return;
			}
		}
		if (inputType.IsUnsignedInteger())
		{
			unsigned int value = 0;
			if (ToAny::GetAsUnsignedInteger(value, stringValue))
			{
				m_Type = DataTypeID::Bool;
				m_Value = value != 0u;
				return;
			}
		}
		if (inputType.IsFloat())
		{
			double value = 0.0;
			if (ToAny::GetAsFloat(value, stringValue))
			{
				m_Type = DataTypeID::Bool;
				m_Value = value != 0.0;
				return;
			}
		}
		if (inputType.IsString())
		{
			m_Type = DataTypeID::Bool;
			m_Value = !stringValue.IsEmpty();
			return;
		}
	}
	void ItemValue::DeserializeAsSignedInteger(TypeID inputType, const wxString& stringValue)
	{
		if (inputType.IsSignedInteger())
		{
			int64_t value = 0;
			if (ToAny::GetAsSignedInteger(value, stringValue))
			{
				m_Type = DataTypeID::Int64;
				m_Value = value;
				return;
			}
		}
		if (inputType.IsUnsignedInteger())
		{
			uint64_t value = 0;
			if (ToAny::GetAsUnsignedInteger(value, stringValue))
			{
				m_Type = DataTypeID::Int64;
				m_Value = value;
				return;
			}
		}
		if (inputType.IsFloat())
		{
			double value = 0.0;
			if (ToAny::GetAsFloat(value, stringValue))
			{
				m_Type = DataTypeID::Int64;
				m_Value = static_cast<int64_t>(value);
				return;
			}
		}
		if (inputType.IsBool())
		{
			bool value = false;
			if (ToAny::GetAsBool(value, stringValue))
			{
				m_Type = DataTypeID::Int32;
				m_Value = static_cast<int>(value);
				return;
			}
		}
		if (inputType.IsString())
		{
			m_Type = DataTypeID::Int32;
			m_Value = static_cast<int>(!stringValue.IsEmpty());
			return;
		}
	}
	void ItemValue::DeserializeAsUnsignedInteger(TypeID inputType, const wxString& stringValue)
	{
		// Should be harmless
		DeserializeAsSignedInteger(inputType, stringValue);
	}
	void ItemValue::DeserializeAsFloat(TypeID inputType, const wxString& stringValue)
	{
		if (inputType.IsFloat())
		{
			double value = 0.0;
			if (ToAny::GetAsFloat(value, stringValue))
			{
				m_Type = DataTypeID::Float32;
				m_Value = value;
				return;
			}
		}
		if (inputType.IsSignedInteger())
		{
			int64_t value = 0;
			if (ToAny::GetAsSignedInteger(value, stringValue))
			{
				m_Type = DataTypeID::Float64;
				m_Value = static_cast<double>(value);
				return;
			}
		}
		if (inputType.IsUnsignedInteger())
		{
			uint64_t value = 0;
			if (ToAny::GetAsUnsignedInteger(value, stringValue))
			{
				m_Type = DataTypeID::Float64;
				m_Value = static_cast<double>(value);
				return;
			}
		}
		if (inputType.IsBool())
		{
			bool value = false;
			if (ToAny::GetAsBool(value, stringValue))
			{
				m_Type = DataTypeID::Float64;
				m_Value = static_cast<double>(value);
				return;
			}
		}
		if (inputType.IsString())
		{
			m_Type = DataTypeID::Float64;
			m_Value = static_cast<double>(!stringValue.IsEmpty());
			return;
		}
	}
	void ItemValue::DeserializeAsString(TypeID inputType, const wxString& stringValue)
	{
		// There's no point in checking anything, just copy raw value
		m_Type = DataTypeID::String;
		m_Value = stringValue;
	}

	wxString ItemValue::DoSerialize(const Item& item) const
	{
		const TypeID type = item.GetTypeID();
		const TypeID outputType = item.GetDataType().GetInputType();

		if (type.IsBool())
		{
			return SerializeFromBool(outputType, item);
		}
		else if (type.IsSignedInteger())
		{
			return SerializeFromSignedInteger(outputType, item);
		}
		else if (type.IsUnsignedInteger())
		{
			return SerializeFromUnsignedInteger(outputType, item);
		}
		else if (type.IsFloat())
		{
			return SerializeFromFloat(outputType, item);
		}
		else if (type.IsString())
		{
			return SerializeFromString(outputType, item);
		}
		return {};
	}
	wxString ItemValue::SerializeFromBool(TypeID outputType, const Item& item) const
	{
		if (outputType.IsBool() || outputType.IsString())
		{
			return FromAny::AsBool(m_Value, item);
		}
		if (outputType.IsInteger())
		{
			return FromAny::AsSignedInteger(m_Value, item);
		}
		if (outputType.IsFloat())
		{
			return FromAny::AsFloat32(m_Value, item);
		}
		return {};
	}
	wxString ItemValue::SerializeFromSignedInteger(TypeID outputType, const Item& item) const
	{
		if (outputType.IsInteger() || outputType.IsBool())
		{
			return FromAny::AsSignedInteger(m_Value, item);
		}
		if (outputType.IsType(DataTypeID::Float32))
		{
			return FromAny::AsFloat32(m_Value, item);
		}
		if (outputType.IsType(DataTypeID::Float64))
		{
			return FromAny::AsFloat64(m_Value, item);
		}
		if (outputType.IsString())
		{
			return FromAny::AsString(m_Value, item);
		}
		return {};
	}
	wxString ItemValue::SerializeFromUnsignedInteger(TypeID outputType, const Item& item) const
	{
		if (outputType.IsInteger() || outputType.IsBool())
		{
			return FromAny::AsUnsignedInteger(m_Value, item);
		}
		if (outputType.IsType(DataTypeID::Float32))
		{
			return FromAny::AsFloat32(m_Value, item);
		}
		if (outputType.IsType(DataTypeID::Float64))
		{
			return FromAny::AsFloat64(m_Value, item);
		}
		if (outputType.IsString())
		{
			return FromAny::AsString(m_Value, item);
		}
		return {};
	}
	wxString ItemValue::SerializeFromFloat(TypeID outputType, const Item& item) const
	{
		if (outputType.IsType(DataTypeID::Float32) || outputType.IsString())
		{
			return FromAny::AsFloat32(m_Value, item);
		}
		if (outputType.IsType(DataTypeID::Float64) || outputType.IsString())
		{
			return FromAny::AsFloat64(m_Value, item);
		}
		if (outputType.IsInteger() || outputType.IsBool())
		{
			return FromAny::AsFloat32(m_Value, item);
		}
		return {};
	}
	wxString ItemValue::SerializeFromString(TypeID outputType, const Item& item) const
	{
		if (outputType.IsString())
		{
			return GetAndFormat<wxString>(m_Value, item);
		}
		if (outputType.IsFloat())
		{
			wxString value = GetAs<wxString>(m_Value, item);
			return FormatAs<double>(!value.IsEmpty(), item);
		}
		if (outputType.IsInteger() || outputType.IsBool())
		{
			wxString value = GetAs<wxString>(m_Value, item);
			return FormatAs<int>(!value.IsEmpty(), item);
		}
		return {};
	}

	wxString ItemValue::Serialize(const Item& item) const
	{
		return IsNull() ? wxString() : DoSerialize(item);
	}
	bool ItemValue::Deserialize(const wxString& value, const Item& item)
	{
		DoDeserialize(value, item);
		return !IsNull();
	}
}
