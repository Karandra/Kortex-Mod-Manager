#pragma once
#include "stdafx.h"
#include "KVariablesTableValue.h"

class KIVariableValue
{
	public:
		using Override = KIVariableValueNS::Override;
		using Type = KIVariableValueNS::Type;

	private:
		wxString m_Value;
		Type m_Type = Type::DoNotChange;
		Override m_Override = Override::DoNotChange;

	public:
		KIVariableValue(const wxString& value = wxEmptyString, Override isOverride = Override::DoNotChange, Type type = Type::DoNotChange)
			:m_Value(value), m_Override(isOverride), m_Type(type)
		{
		}
		KIVariableValue(const wchar_t* value, Override isOverride = Override::DoNotChange, Type type = Type::DoNotChange)
			:m_Value(value), m_Override(isOverride), m_Type(type)
		{
		}
		KIVariableValue(const char* value, Override isOverride = Override::DoNotChange, Type type = Type::DoNotChange)
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

class KIVariablesTable
{
	public:
		using Visitor = std::function<bool(const wxString&, const KIVariableValue&)>;

	public:
		KIVariablesTable() = default;
		virtual ~KIVariablesTable() = default;

	public:
		virtual bool IsEmpty() const = 0;

		virtual bool HasVariable(const wxString& id) const = 0;
		virtual KIVariableValue GetVariable(const wxString& id) const = 0;
		virtual void SetVariable(const wxString& id, const KIVariableValue& value) = 0;

		virtual void Accept(const Visitor& visitor) const = 0;

		// This function supports $(...), $T(...) and $SH(...) syntax. Variable name (...) can contain spaces.
		// $(...) normal variable like $(AppName).
		// $T(...) localization variable such as $T(Generic.Run) or $T(ID_OK).
		// $SH(...) shell variable like $SH(SHF_DESKTOP). Currently only supports expansion of constants
		// from 'KxShellFolderID' enum (without 'Kx' prefix).
		wxString Expand(const wxString& variables) const;
};
