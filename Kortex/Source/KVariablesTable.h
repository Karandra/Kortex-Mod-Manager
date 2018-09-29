#pragma once
#include "stdafx.h"

class KIVariablesTable
{
	public:
		KIVariablesTable() = default;
		virtual ~KIVariablesTable() = default;

	public:
		virtual bool IsEmpty() const = 0;

		virtual bool HasVariable(const wxString& id) const = 0;
		virtual wxString GetVariable(const wxString& id) const = 0;
		virtual void SetVariable(const wxString& id, const wxString& value) = 0;

		// This function supports $(...), $T(...) and $SH(...) syntax. Variable name (...) can contain spaces.
		// $(...) normal variable like $(AppName).
		// $T(...) localization variable such as $T(Generic.Run) or $T(ID_OK).
		// $SH(...) shell variable like $SH(SHF_DESKTOP). Currently only supports expansion of constants
		// from 'KxShellFolderID' enum (without 'Kx' prefix).
		wxString Expand(const wxString& variables) const;
};
