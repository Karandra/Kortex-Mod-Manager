#pragma once
#include "stdafx.h"

namespace Kortex::VariablesTable
{
	enum class Override
	{
		False = 0,
		True = 1,
	};
	enum class Type
	{
		String,
		Integer,
		FSPath,
	};
}

namespace Kortex
{
	class VariableValue
	{
		public:
			using Override = VariablesTable::Override;
			using Type = VariablesTable::Type;

		private:
			wxString m_Value;
			std::optional<Type> m_Type = Type::String;
			std::optional<Override> m_Override;

		private:
			template<class T> static T ToInt(const wxString& value, T defaultValue)
			{
				using Tint = std::conditional_t<std::is_unsigned_v<T>, wxULongLong_t, wxLongLong_t>;
				Tint intValue = static_cast<Tint>(defaultValue);

				if constexpr(std::is_unsigned_v<T>)
				{
					if (m_Value.ToULongLong(&intValue))
					{
						return static_cast<T>(intValue);
					}
				}
				else
				{
					if (m_Value.ToLongLong(&intValue))
					{
						return static_cast<T>(intValue);
					}
				}
				return defaultValue;
			}
			template<class T> static T ToFloat(const wxString& value, T defaultValue)
			{
				double floatValue = static_cast<double>(defaultValue);
				if (value.ToCDouble(&floatValue))
				{
					return static_cast<T>(floatValue);
				}
				return defaultValue;
			}

		public:
			VariableValue() = default;
			template<class T> VariableValue(const T& value, std::optional<Override> isOverride = {}, std::optional<Type> type = {})
				:m_Override(std::move(isOverride)), m_Type(std::move(type))
			{
				Assign(value);
			}

		public:
			bool IsOK() const
			{
				return !m_Value.IsEmpty();
			}

			wxString Expand() const;
			template<class T> void Assign(const T& value)
			{
				if constexpr(std::is_convertible_v<T, wxString> || std::is_assignable_v<wxString, T>)
				{
					m_Value = value;
				}
				else
				{
					m_Value = KxString::Format(wxS("%1"), value);
				}
			}

			const wxString& AsString() const
			{
				return m_Value;
			}
			wxString ExpandAsString() const
			{
				return Expand();
			}
			
			template<class T = int> T AsInt(T defaultValue = 0) const
			{
				return ToInt(m_Value, defaultValue);
			}
			template<class T = int> T ExpandAsInt(T defaultValue = 0) const
			{
				return ToInt(Expand(), defaultValue);
			}
			
			template<class T = double> T AsFloat(T defaultValue = 0) const
			{
				return ToFloat(m_Value, defaultValue);
			}
			template<class T = double> T ExpandAsFloat(T defaultValue = 0) const
			{
				return ToFloat(Expand(), defaultValue);
			}

			std::optional<Override> GetRawOverride() const
			{
				return m_Override;
			}
			bool IsOverride() const
			{
				return m_Override && m_Override == Override::True;
			}
			void SetOverride(bool value)
			{
				m_Override = value ? Override::True : Override::False;
			}

			std::optional<Type> GetRawType() const
			{
				return m_Type;
			}
			Type GetType() const
			{
				return m_Type ? *m_Type : Type::String;
			}
			void SetType(Type type)
			{
				m_Type = type;
			}

		public:
			explicit operator bool() const
			{
				return IsOK();
			}
			bool operator!() const
			{
				return !IsOK();
			}
			
			operator const wxString&() const
			{
				return m_Value;
			}
	};
}
