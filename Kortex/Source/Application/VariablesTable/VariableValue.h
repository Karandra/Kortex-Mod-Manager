#pragma once
#include "stdafx.h"
#include "VariableValueInternals.h"

namespace Kortex
{
	class VariableValue
	{
		public:
			using Override = VariablesTable::Internal::Override;
			using Type = VariablesTable::Internal::Type;

		private:
			wxString m_Value;
			Type m_Type = Type::DoNotChange;
			Override m_Override = Override::DoNotChange;

		public:
			VariableValue(const wxString& value = wxEmptyString, Override isOverride = Override::DoNotChange, Type type = Type::DoNotChange)
				:m_Value(value), m_Override(isOverride), m_Type(type)
			{
			}
			VariableValue(const wchar_t* value, Override isOverride = Override::DoNotChange, Type type = Type::DoNotChange)
				:m_Value(value), m_Override(isOverride), m_Type(type)
			{
			}
			VariableValue(const char* value, Override isOverride = Override::DoNotChange, Type type = Type::DoNotChange)
				:m_Value(value), m_Override(isOverride), m_Type(type)
			{
			}

		public:
			const wxString& GetValue() const
			{
				return m_Value;
			}
			void SetValue(const wxString& value)
			{
				m_Value = value;
			}

			Override GetOverride() const
			{
				return m_Override;
			}
			bool IsOverride() const
			{
				return m_Override == Override::True;
			}
			void SetOverride(Override value)
			{
				m_Override = value;
			}
			void SetOverride(bool value)
			{
				m_Override = value;
			}

			Type GetType() const
			{
				return m_Type;
			}
			void SetType(Type type)
			{
				m_Type = type;
			}

			operator const wxString&() const
			{
				return m_Value;
			}
	};
}
