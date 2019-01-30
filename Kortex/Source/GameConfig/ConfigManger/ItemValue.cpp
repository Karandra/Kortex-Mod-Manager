#include "stdafx.h"
#include "ItemValue.h"
#include "ItemOptions.h"
#include <KxFramework/KxString.h>

namespace
{
	using Kortex::GameConfig::ItemOptions;

	template<class T> wxString GetAs(const wxAny& anyValue, const ItemOptions& options)
	{
		T value;
		if (anyValue.GetAs(&value))
		{
			return value;
		}
		return {};
	}
	template<class T> wxString FormatAs(const T& value, const ItemOptions& options)
	{
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
	template<class T> wxString GetAndFormat(const wxAny& anyValue, const ItemOptions& options)
	{
		T value;
		if (anyValue.GetAs(&value))
		{
			return FormatAs(value, options);
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
		wxString AsBool(const wxAny& value, const ItemOptions& options)
		{
			bool boolValue = false;
			value.GetAs(&boolValue);

			return boolValue ? wxS("true") : wxS("false");
		}
		wxString AsSignedInteger(const wxAny& value, const ItemOptions& options)
		{
			return GetAndFormat<int64_t>(value, options);
		}
		wxString AsUnsignedInteger(const wxAny& value, const ItemOptions& options)
		{
			return GetAndFormat<uint64_t>(value, options);
		}
		wxString AsFloat32(const wxAny& value, const ItemOptions& options)
		{
			return GetAndFormat<float>(value, options);
		}
		wxString AsFloat64(const wxAny& value, const ItemOptions& options)
		{
			return GetAndFormat<double>(value, options);
		}
		wxString AsString(const wxAny& value, const ItemOptions& options)
		{
			return GetAndFormat<wxString>(value, options);
		}
	}
}

namespace Kortex::GameConfig
{
	void ItemValue::FromString(const wxString& stringValue, const ItemOptions& options)
	{
		const TypeID type = m_Type.GetID();
		const TypeID inputType = m_Type.GetInputType();

		if (type.IsBool())
		{
			AsBool(inputType, stringValue);
		}
		else if (type.IsSignedInteger())
		{
			AsSignedInteger(inputType, stringValue);
		}
		else if (type.IsUnsignedInteger())
		{
			AsUnsignedInteger(inputType, stringValue);
		}
		else if (type.IsFloat())
		{
			AsFloat(inputType, stringValue);
		}
		else if (type.IsString())
		{
			AsString(inputType, stringValue);
		}
	}
	void ItemValue::AsBool(TypeID inputType, const wxString& stringValue)
	{
		if (inputType.IsBool())
		{
			bool value = false;
			if (ToAny::GetAsBool(value, stringValue))
			{
				m_Value = value;
				return;
			}
		}
		if (inputType.IsSignedInteger())
		{
			int value = 0;
			if (ToAny::GetAsSignedInteger(value, stringValue))
			{
				m_Value = value != 0;
				return;
			}
		}
		if (inputType.IsUnsignedInteger())
		{
			unsigned int value = 0;
			if (ToAny::GetAsUnsignedInteger(value, stringValue))
			{
				m_Value = value != 0u;
				return;
			}
		}
		if (inputType.IsFloat())
		{
			double value = 0.0;
			if (ToAny::GetAsFloat(value, stringValue))
			{
				m_Value = value != 0.0;
				return;
			}
		}
		if (inputType.IsString())
		{
			m_Value = !stringValue.IsEmpty();
			return;
		}
	}
	void ItemValue::AsSignedInteger(TypeID inputType, const wxString& stringValue)
	{
		if (inputType.IsSignedInteger())
		{
			int64_t value = 0;
			if (ToAny::GetAsSignedInteger(value, stringValue))
			{
				m_Value = value;
				return;
			}
		}
		if (inputType.IsUnsignedInteger())
		{
			uint64_t value = 0;
			if (ToAny::GetAsUnsignedInteger(value, stringValue))
			{
				m_Value = value;
				return;
			}
		}
		if (inputType.IsFloat())
		{
			double value = 0.0;
			if (ToAny::GetAsFloat(value, stringValue))
			{
				m_Value = value;
				return;
			}
		}
		if (inputType.IsBool())
		{
			bool value = false;
			if (ToAny::GetAsBool(value, stringValue))
			{
				m_Value = (int)value;
				return;
			}
		}
		if (inputType.IsString())
		{
			m_Value = (int)!stringValue.IsEmpty();
			return;
		}
	}
	void ItemValue::AsUnsignedInteger(TypeID inputType, const wxString& stringValue)
	{
		// Should be harmless
		AsSignedInteger(inputType, stringValue);
	}
	void ItemValue::AsFloat(TypeID inputType, const wxString& stringValue)
	{
		if (inputType.IsFloat())
		{
			double value = 0.0;
			if (ToAny::GetAsFloat(value, stringValue))
			{
				m_Value = value;
				return;
			}
		}
		if (inputType.IsSignedInteger())
		{
			int64_t value = 0;
			if (ToAny::GetAsSignedInteger(value, stringValue))
			{
				m_Value = static_cast<double>(value);
				return;
			}
		}
		if (inputType.IsUnsignedInteger())
		{
			uint64_t value = 0;
			if (ToAny::GetAsUnsignedInteger(value, stringValue))
			{
				m_Value = static_cast<double>(value);
				return;
			}
		}
		if (inputType.IsBool())
		{
			bool value = false;
			if (ToAny::GetAsBool(value, stringValue))
			{
				m_Value = static_cast<double>(value);
				return;
			}
		}
		if (inputType.IsString())
		{
			m_Value = static_cast<double>(!stringValue.IsEmpty());
			return;
		}
	}
	void ItemValue::AsString(TypeID inputType, const wxString& stringValue)
	{
		// There's no point in checking anything, just copy raw value
		m_Value = stringValue;
	}

	wxString ItemValue::ToString(const ItemOptions& options) const
	{
		const TypeID type = m_Type.GetID();
		const TypeID outputType = m_Type.GetInputType();

		if (type.IsBool())
		{
			return FromBool(outputType, options);
		}
		else if (type.IsSignedInteger())
		{
			return FromSignedInteger(outputType, options);
		}
		else if (type.IsUnsignedInteger())
		{
			return FromUnsignedInteger(outputType, options);
		}
		else if (type.IsFloat())
		{
			return FromFloat(outputType, options);
		}
		else if (type.IsString())
		{
			FromString(outputType, options);
		}
	}
	wxString ItemValue::FromBool(TypeID outputType, const ItemOptions& options) const
	{
		if (outputType.IsBool() || outputType.IsString())
		{
			return FromAny::AsBool(m_Value, options);
		}
		if (outputType.IsInteger())
		{
			return FromAny::AsSignedInteger(m_Value, options);
		}
		if (outputType.IsFloat())
		{
			return FromAny::AsFloat32(m_Value, options);
		}
		return {};
	}
	wxString ItemValue::FromSignedInteger(TypeID outputType, const ItemOptions& options) const
	{
		if (outputType.IsInteger() || outputType.IsBool())
		{
			return FromAny::AsSignedInteger(m_Value, options);
		}
		if (outputType.IsType(DataTypeID::Float32))
		{
			return FromAny::AsFloat32(m_Value, options);
		}
		if (outputType.IsType(DataTypeID::Float64))
		{
			return FromAny::AsFloat64(m_Value, options);
		}
		if (outputType.IsString())
		{
			return FromAny::AsString(m_Value, options);
		}
		return {};
	}
	wxString ItemValue::FromUnsignedInteger(TypeID outputType, const ItemOptions& options) const
	{
		if (outputType.IsInteger() || outputType.IsBool())
		{
			return FromAny::AsUnsignedInteger(m_Value, options);
		}
		if (outputType.IsType(DataTypeID::Float32))
		{
			return FromAny::AsFloat32(m_Value, options);
		}
		if (outputType.IsType(DataTypeID::Float64))
		{
			return FromAny::AsFloat64(m_Value, options);
		}
		if (outputType.IsString())
		{
			return FromAny::AsString(m_Value, options);
		}
		return {};
	}
	wxString ItemValue::FromFloat(TypeID outputType, const ItemOptions& options) const
	{
		if (outputType.IsType(DataTypeID::Float32) || outputType.IsString())
		{
			return FromAny::AsFloat32(m_Value, options);
		}
		if (outputType.IsType(DataTypeID::Float64) || outputType.IsString())
		{
			return FromAny::AsFloat64(m_Value, options);
		}
		if (outputType.IsInteger() || outputType.IsBool())
		{
			return FromAny::AsFloat32(m_Value, options);
		}
		return {};
	}
	wxString ItemValue::FromString(TypeID outputType, const ItemOptions& options) const
	{
		if (outputType.IsString())
		{
			return GetAndFormat<wxString>(m_Value, options);
		}
		if (outputType.IsFloat())
		{
			wxString value = GetAs<wxString>(m_Value, options);
			return FormatAs<double>(!value.IsEmpty(), options);
		}
		if (outputType.IsInteger() || outputType.IsBool())
		{
			wxString value = GetAs<wxString>(m_Value, options);
			return FormatAs<int>(!value.IsEmpty(), options);
		}
		return {};
	}
}
