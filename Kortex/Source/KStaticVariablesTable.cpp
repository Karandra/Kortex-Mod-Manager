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
	auto it = m_StaticVariables.find(id);
	if (it != m_StaticVariables.end())
	{
		KIVariableValue& variable = it->second;

		variable.SetValue(value.GetValue());
		if (value.GetOverride() != KIVariableValue::Override::DoNotChange)
		{
			variable.SetOverride(value.GetOverride());
		}
		if (value.GetType() != KIVariableValue::Type::DoNotChange)
		{
			variable.SetType(value.GetType());
		}
	}
	else
	{
		m_StaticVariables.insert_or_assign(id, KIVariableValue(value.GetValue(), KIVariableValue::Override::False, KIVariableValue::Type::None));
	}
}
