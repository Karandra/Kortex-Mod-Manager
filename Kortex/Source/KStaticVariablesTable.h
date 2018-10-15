#pragma once
#include "stdafx.h"
#include "KVariablesTable.h"

// This is static variables table. It can return values that explicitly saved there
// with only exception of 'Expand' function which can get some variables dynamically.
// If you need more flexible behavior take a look at 'KDynamicVariablesTable'.
class KStaticVariablesTable: public KIVariablesTable
{
	protected:
		std::unordered_map<wxString, KIVariableValue> m_StaticVariables;

	protected:
		bool IterateOverStatic(const Visitor& visitor) const;

	public:
		virtual bool IsEmpty() const;

		virtual bool HasVariable(const wxString& id) const override;
		virtual KIVariableValue GetVariable(const wxString& id) const override;
		virtual void SetVariable(const wxString& id, const KIVariableValue& value) override;

		virtual void Accept(const Visitor& visitor) const override
		{
			IterateOverStatic(visitor);
		}
};