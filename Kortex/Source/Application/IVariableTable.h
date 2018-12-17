#pragma once
#include "stdafx.h"
#include "VariablesTable/VariableValue.h"

namespace Kortex
{
	class IVariableTable
	{
		public:
			using Visitor = std::function<bool(const wxString&, const VariableValue&)>;

		public:
			IVariableTable() = default;
			virtual ~IVariableTable() = default;

		public:
			virtual bool IsEmpty() const = 0;

			virtual bool HasVariable(const wxString& id) const = 0;
			virtual VariableValue GetVariable(const wxString& id) const = 0;
			virtual void SetVariable(const wxString& id, const VariableValue& value) = 0;

			virtual void Accept(const Visitor& visitor) const = 0;

			// This function supports $(...), $T(...) and $SHF(...) syntax. Variable name (...) can contain spaces.
			// $(...) normal variable like $(AppName).
			// $T(...) localization variable such as $T(Generic.Run) or $T(ID_OK).
			// $SHF(...) shell variable like $SHF(DESKTOP). Currently only supports expansion of constants
			// from 'KxShellFolderID' enum (without 'Kx' prefix).
			wxString Expand(const wxString& variables) const;
	};
}
