#include "stdafx.h"
#include "StaticVariableTable.h"

namespace Kortex
{
	bool StaticVariableTable::IterateOverStatic(const Visitor& visitor) const
	{
		for (const auto& item: m_StaticVariables)
		{
			if (!visitor(item.first, item.second))
			{
				return false;
			}
		}
		return true;
	}

	bool StaticVariableTable::IsEmpty() const
	{
		return m_StaticVariables.empty();
	}

	bool StaticVariableTable::HasVariable(const wxString& id) const
	{
		return m_StaticVariables.find(id) != m_StaticVariables.end();
	}
	VariableValue StaticVariableTable::GetVariable(const wxString& id) const
	{
		auto it = m_StaticVariables.find(id);
		if (it != m_StaticVariables.end())
		{
			return it->second;
		}
		return {};
	}
	void StaticVariableTable::SetVariable(const wxString& id, const VariableValue& value)
	{
		auto it = m_StaticVariables.find(id);
		if (it != m_StaticVariables.end())
		{
			VariableValue& variable = it->second;

			// Change variable value
			variable.Assign(value.AsString());

			// Change attributes if we have asked to
			if (value.GetRawOverride())
			{
				variable.SetOverride(value.IsOverride());
			}
			if (value.GetRawType())
			{
				variable.SetType(value.GetType());
			}
		}
		else
		{
			m_StaticVariables.insert_or_assign(id, value);
		}
	}
}
