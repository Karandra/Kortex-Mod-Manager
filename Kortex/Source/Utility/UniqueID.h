#pragma once
#include "stdafx.h"
#include <sstream>

namespace Kortex::Utility::UniqueID
{
	template<class T, T t_InvalidValue, bool t_AllowNegative, class TTag>
	class IntegerID
	{
		public:
			using TValue = typename T;

		public:
			constexpr static T GetInvalidValue()
			{
				return t_InvalidValue;
			}

		private:
			std::optional<TValue> m_Value;

		private:
			bool CheckAndAssign(TValue value)
			{
				if constexpr (t_AllowNegative)
				{
					if (value != t_InvalidValue)
					{
						m_Value = value;
						return true;
					}
				}
				else
				{
					if (value != t_InvalidValue && value > 0)
					{
						m_Value = value;
						return true;
					}
				}
				return false;
			}

		public:
			IntegerID() = default;
			IntegerID(TValue value)
			{
				static_assert(std::is_integral_v<TValue>, "IntegerID supports only integral types");
				static_assert(std::is_signed_v<TValue>, "IntegerID supports only signed integers");
				
				CheckAndAssign(value);
			}
			IntegerID(const wxString& value)
			{
				FromString(value);
			}
			IntegerID(IntegerID&& other)
				:m_Value(std::move(other.m_Value))
			{
			}
			IntegerID(const IntegerID& other)
				:m_Value(other.m_Value)
			{
			}

			IntegerID& operator=(TValue value)
			{
				CheckAndAssign(value);
				return *this;
			}
			IntegerID& operator=(const wxString& value)
			{
				FromString(value);
				return *this;
			}
			IntegerID& operator=(IntegerID&& other)
			{
				m_Value = std::move(other.m_Value);
				return *this;
			}
			IntegerID& operator=(const IntegerID& other)
			{
				m_Value = other.m_Value;
				return *this;
			}

		public:
			bool HasValue() const
			{
				return m_Value.has_value();
			}
			TValue GetValue() const
			{
				return m_Value ? *m_Value : t_InvalidValue;
			}
			void SetValue(TValue value)
			{
				CheckAndAssign(value);
			}

			wxString ToString() const
			{
				return m_Value ? std::to_wstring(*m_Value) : wxString();
			}
			bool FromString(const wxString& stringValue)
			{
				m_Value.reset();

				std::basic_stringstream<wxChar, std::char_traits<wxChar>, std::allocator<wxChar>> stream;
				TValue value = t_InvalidValue;
				stream << stringValue.wc_str();
				stream >> value;

				if (!stream.fail())
				{
					CheckAndAssign(value);
				}
				return m_Value.has_value();
			}

		public:
			explicit operator bool() const
			{
				return m_Value.has_value();
			}
			bool operator!() const
			{
				return !m_Value.has_value();
			}

			bool operator==(const IntegerID& other) const
			{
				return m_Value == other.m_Value;
			}
			bool operator!=(const IntegerID& other) const
			{
				return !(*this == other);
			}
	};
}
