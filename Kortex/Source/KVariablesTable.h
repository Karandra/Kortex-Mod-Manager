#pragma once
#include "stdafx.h"

enum KVariableTypeEnum
{
	KVT_INVALID,

	KVT_STRING,
	KVT_INT,
	KVT_FLOAT,
	KVT_REGISTRY_ENTRY,
};

class KVariablesTable;
class KVariableValue
{
	friend class KVariablesTable;

	private:
		wxString m_Value;
		KVariableTypeEnum m_Type = KVT_INVALID;

	public:
		KVariableValue(const wxString& value, KVariableTypeEnum type = KVT_STRING)
			:m_Value(value), m_Type(type)
		{
		}

		const wxString& GetValue() const
		{
			return m_Value;
		}
};

//////////////////////////////////////////////////////////////////////////
// KVariablesTable is static variables table. It can return values that explicitly saved there
// with only exception of 'Expand' function which can get some variables dynamically.
// If you need more flexible behavior take a look at 'KDynamicVariablesTable'.
class KVariablesTable
{
	protected:
		std::unordered_map<wxString, KVariableValue> m_Impl;

	public:
		KVariablesTable();
		virtual ~KVariablesTable();

	public:
		virtual bool IsEmpty() const
		{
			return m_Impl.empty();
		}

		virtual bool HasVariable(const wxString& id) const;
		virtual wxString GetVariable(const wxString& id) const;
		virtual void SetVariable(const wxString& id, const wxString& value, KVariableTypeEnum type = KVT_STRING);

		// This function supports $(...), $T(...) and $SH(...) syntax. Variable name (...) can contain spaces.
		// $(...) normal variable like $(AppName).
		// $T(...) localization variable such as $T(Generic.Run) or $T(ID_OK).
		// $SH(...) shell variable like $SH(SHF_DESKTOP). Currently only supports expansion of constants
		// from 'KxShellFolderID' enum (without 'Kx' prefix).
		wxString Expand(const wxString& variables) const;
};