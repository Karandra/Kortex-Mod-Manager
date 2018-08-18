#include "stdafx.h"
#include "KVariablesTable.h"
#include "KDynamicVariablesTable.h"
#include "KApp.h"
#include "KAux.h"

void KDynamicVariablesTable::NewVariable(const wxString& id, const EntryType& functor)
{
	m_DynamicVariables.emplace(std::make_pair(id, functor));
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
	return m_DynamicVariables.empty() && KVariablesTable::IsEmpty();
}

bool KDynamicVariablesTable::HasVariable(const wxString& id) const
{
	return m_DynamicVariables.count(id) || KVariablesTable::HasVariable(id);
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
	return KVariablesTable::GetVariable(id);
}
