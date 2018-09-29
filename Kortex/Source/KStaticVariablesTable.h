#pragma once
#include "stdafx.h"
#include "KVariablesTable.h"

// This is static variables table. It can return values that explicitly saved there
// with only exception of 'Expand' function which can get some variables dynamically.
// If you need more flexible behavior take a look at 'KDynamicVariablesTable'.
class KStaticVariablesTable: public KIVariablesTable
{
	protected:
		std::unordered_map<wxString, wxString> m_StaticVariables;

	public:
		virtual bool IsEmpty() const
		{
			return m_StaticVariables.empty();
		}

		virtual bool HasVariable(const wxString& id) const override
		{
			return m_StaticVariables.count(id);
		}
		virtual wxString GetVariable(const wxString& id) const override
		{
			auto it = m_StaticVariables.find(id);
			if (it != m_StaticVariables.end())
			{
				return it->second;
			}
			else
			{
				return wxEmptyString;
			}
		}
		virtual void SetVariable(const wxString& id, const wxString& value) override
		{
			m_StaticVariables.insert_or_assign(id, value);
		}
};