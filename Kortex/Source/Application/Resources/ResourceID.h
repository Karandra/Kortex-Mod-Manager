#pragma once
#include "stdafx.h"

namespace Kortex
{
	class ResourceID final
	{
		public:
			using TIntValue = int;

			template<class T> static constexpr bool TestIntType()
			{
				return std::is_integral_v<T> || std::is_enum_v<T>;
			}

		public:
			std::variant<wxString, TIntValue> m_Value;

		public:
			ResourceID() = default;

			template<class T = TIntValue, class = std::enable_if_t<TestIntType<T>()>>
			ResourceID(T value) noexcept
				:m_Value(static_cast<TIntValue>(value))
			{
			}

			ResourceID(const wxString& value) noexcept
				:m_Value(value)
			{
			}
			ResourceID(const char* value) noexcept
				:m_Value(value)
			{
			}
			ResourceID(const wchar_t* value) noexcept
				:m_Value(value)
			{
			}

			ResourceID(ResourceID&&) = default;
			ResourceID(const ResourceID&) = default;

		public:
			template<class T = TIntValue, class = std::enable_if_t<TestIntType<T>()>>
			ResourceID& operator=(T value) noexcept
			{
				m_Value = static_cast<TIntValue>(value);
				return *this;
			}

			ResourceID& operator=(const wxString& value) noexcept
			{
				m_Value = value;
				return *this;
			}
			ResourceID& operator=(const char* value) noexcept
			{
				m_Value = value;
				return *this;
			}
			ResourceID& operator=(const wchar_t* value) noexcept
			{
				m_Value = value;
				return *this;
			}

			ResourceID& operator=(ResourceID&&) = default;
			ResourceID& operator=(const ResourceID&) = default;

		public:
			bool IsOK() const
			{
				return IsInteger() || IsString();
			}
			explicit operator bool() const
			{
				return IsOK();
			}
			bool operator!() const
			{
				return !IsOK();
			}
			
			bool IsInteger() const
			{
				return std::holds_alternative<TIntValue>(m_Value);
			}
			bool IsString() const
			{
				return TryAsString().has_value();
			}

			std::optional<std::reference_wrapper<const wxString>> TryAsString() const
			{
				const wxString* value = std::get_if<wxString>(&m_Value);
				if (value && !value->IsEmpty())
				{
					return *value;
				}
				return std::nullopt;
			}
			std::optional<TIntValue> TryAsInt() const
			{
				if (const TIntValue* value = std::get_if<int>(&m_Value))
				{
					return *value;
				}
				return std::nullopt;
			}

			wxString AsString(const wxString& defaultValue = {}) const
			{
				if (auto value = TryAsString())
				{
					return *value;
				}
				return defaultValue;
			}
			template<class T = TIntValue> T AsInt(T defaultValue = -1) const
			{
				if (auto value = TryAsInt())
				{
					return static_cast<T>(*value);
				}
				return defaultValue;
			}
	};
}
