#include "stdafx.h"
#include "KDynamicVariablesTable.h"
#include "KAux.h"

void KDynamicVariablesTable::DoSetDynamicVariable(const wxString& id, const VariableFunctor& functor)
{
	m_DynamicVariables._Insert_or_assign(id, functor);
}
bool KDynamicVariablesTable::IterateOverDynamyc(const Visitor& visitor) const
{
	for (const auto& item: m_DynamicVariables)
	{
		if (!visitor(item.first, item.second()))
		{
			return false;
		}
	}
	return true;
}

KDynamicVariablesTable::KDynamicVariablesTable()
{
	DoSetDynamicVariable(wxS("CurrentDate"), []()
	{
		return KAux::FormatDate(wxDateTime::Now());
	});
	DoSetDynamicVariable(wxS("CurrentTime"), []()
	{
		return KAux::FormatTime(wxDateTime::Now());
	});
	DoSetDynamicVariable(wxS("CurrentDateTime"), []()
	{
		return KAux::FormatDateTime(wxDateTime::Now());
	});
}
KDynamicVariablesTable::~KDynamicVariablesTable()
{
}

bool KDynamicVariablesTable::IsEmpty() const
{
	return m_DynamicVariables.empty() && KStaticVariablesTable::IsEmpty();
}

bool KDynamicVariablesTable::HasVariable(const wxString& id) const
{
	return m_DynamicVariables.count(id) || KStaticVariablesTable::HasVariable(id);
}
KIVariableValue KDynamicVariablesTable::GetVariable(const wxString& id) const
{
	// Do not use our 'KDynamicVariablesTable::HasVariable' here. It's only for user
	// not for internal use.

	auto it = m_DynamicVariables.find(id);
	if (it != m_DynamicVariables.end())
	{
		return it->second();
	}
	return KStaticVariablesTable::GetVariable(id);
}
void KDynamicVariablesTable::SetVariable(const wxString& id, const KIVariableValue& value)
{
	// Add new only if this is not one of dynamic variables
	if (m_DynamicVariables.count(id) == 0)
	{
		KStaticVariablesTable::SetVariable(id, value);
	}
}
void KDynamicVariablesTable::SetDynamicVariable(const wxString& id, const VariableFunctor& functor)
{
	if (m_StaticVariables.count(id) == 0)
	{
		DoSetDynamicVariable(id, functor);
	}
}

void KDynamicVariablesTable::Accept(const Visitor& visitor) const
{
	if (IterateOverStatic(visitor))
	{
		IterateOverDynamyc(visitor);
	}
}
