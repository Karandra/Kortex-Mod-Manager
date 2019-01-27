#include "stdafx.h"
#include "ItemValue.h"

namespace
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

namespace Kortex::GameConfig
{
	void ItemValue::FromString(const wxString& stringValue)
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
			if (GetAsBool(value, stringValue))
			{
				m_Value = value;
				return;
			}
		}
		if (inputType.IsSignedInteger())
		{
			int value = 0;
			if (GetAsSignedInteger(value, stringValue))
			{
				m_Value = value != 0;
				return;
			}
		}
		if (inputType.IsUnsignedInteger())
		{
			unsigned int value = 0;
			if (GetAsUnsignedInteger(value, stringValue))
			{
				m_Value = value != 0u;
				return;
			}
		}
		if (inputType.IsFloat())
		{
			double value = 0.0;
			if (GetAsFloat(value, stringValue))
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
			if (GetAsSignedInteger(value, stringValue))
			{
				m_Value = value;
				return;
			}
		}
		if (inputType.IsUnsignedInteger())
		{
			uint64_t value = 0;
			if (GetAsUnsignedInteger(value, stringValue))
			{
				m_Value = value;
				return;
			}
		}
		if (inputType.IsFloat())
		{
			double value = 0.0;
			if (GetAsFloat(value, stringValue))
			{
				m_Value = value;
				return;
			}
		}
		if (inputType.IsBool())
		{
			bool value = false;
			if (GetAsBool(value, stringValue))
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
			if (GetAsFloat(value, stringValue))
			{
				m_Value = value;
				return;
			}
		}
		if (inputType.IsSignedInteger())
		{
			int64_t value = 0;
			if (GetAsSignedInteger(value, stringValue))
			{
				m_Value = value;
				return;
			}
		}
		if (inputType.IsUnsignedInteger())
		{
			uint64_t value = 0;
			if (GetAsUnsignedInteger(value, stringValue))
			{
				m_Value = value;
				return;
			}
		}
		if (inputType.IsBool())
		{
			bool value = false;
			if (GetAsBool(value, stringValue))
			{
				m_Value = (double)value;
				return;
			}
		}
		if (inputType.IsString())
		{
			m_Value = (double)!stringValue.IsEmpty();
			return;
		}
	}
	void ItemValue::AsString(TypeID inputType, const wxString& stringValue)
	{
		// There's no point in checking anything, just copy raw value
		m_Value = stringValue;
	}
}
