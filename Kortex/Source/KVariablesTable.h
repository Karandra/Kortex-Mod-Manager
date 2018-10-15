#pragma once
#include "stdafx.h"

class KIVariableValue
{
	public:
		class Override
		{
			public:
				enum _Enum: int
				{
					False = 0,
					True = 1,
					DoNotChange = 2
				};

			private:
				_Enum m_Value = _Enum::DoNotChange;

			public:
				Override()
					:m_Value(_Enum::DoNotChange)
				{
				}
				Override(_Enum value)
					:m_Value(value)
				{
				}
				Override(bool value)
					:m_Value(value ? _Enum::True : _Enum::False)
				{
				}
		
			public:
				operator _Enum() const
				{
					return m_Value;
				}
		};

	private:
		wxString m_Value;
		Override m_IsOverride = Override::DoNotChange;

	public:
		KIVariableValue(const wxString& value = wxEmptyString, Override isOverride = Override::DoNotChange)
			:m_Value(value), m_IsOverride(isOverride)
		{
		}
		KIVariableValue(const wchar_t* value, Override isOverride = Override::DoNotChange)
			:m_Value(value), m_IsOverride(isOverride)
		{
		}
		KIVariableValue(const char* value, Override isOverride = Override::DoNotChange)
			:m_Value(value), m_IsOverride(isOverride)
		{
		}

	public:
		wxString GetValue() const
		{
			return m_Value;
		}
		void SetValue(const wxString& value)
		{
			m_Value = value;
		}

		Override GetOverrideState() const
		{
			return m_IsOverride;
		}
		bool IsOverride() const
		{
			return m_IsOverride == Override::True;
		}
		void SetOverride(bool value)
		{
			m_IsOverride = value;
		}

		operator wxString() const
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
