#include "stdafx.h"
#include "KDynamicVariablesTable.h"
#include "KAux.h"

void KDynamicVariablesTable::NewVariable(const wxString& id, const VariableFunctor& functor)
{
	m_DynamicVariables.emplace(id, functor);
}

KDynamicVariablesTable::KDynamicVariablesTable()
{
	NewVariable("CurrentDate", []()
	{
		return KAux::FormatDate(wxDateTime::Now());
	});

	NewVariable("CurrentTime", []()
	{
		return KAux::FormatTime(wxDateTime::Now());
	});

	NewVariable("CurrentDateTime", []()
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
wxString KDynamicVariablesTable::GetVariable(const wxString& id) const
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
