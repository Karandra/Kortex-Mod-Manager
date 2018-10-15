#include "stdafx.h"
#include "KStaticVariablesTable.h"

bool KStaticVariablesTable::IterateOverStatic(const Visitor& visitor) const
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

bool KStaticVariablesTable::IsEmpty() const
{
	return m_StaticVariables.empty();
}

bool KStaticVariablesTable::HasVariable(const wxString& id) const
{
	return m_StaticVariables.count(id);
}
KIVariableValue KStaticVariablesTable::GetVariable(const wxString& id) const
{
	auto it = m_StaticVariables.find(id);
	if (it != m_StaticVariables.end())
	{
		return it->second;
	}
	return KIVariableValue();
}
void KStaticVariablesTable::SetVariable(const wxString& id, const KIVariableValue& value)
{
	if (value.GetOverrideState() == KIVariableValue::Override::DoNotChange)
	{
		auto it = m_StaticVariables.find(id);
		if (it != m_StaticVariables.end())
		{
			it->second.SetValue(value.GetValue());
		}
		else
		{
			m_StaticVariables.insert_or_assign(id, KIVariableValue(value.GetValue(), false));
		}
		return;
	}
	m_StaticVariables.insert_or_assign(id, value);
}
