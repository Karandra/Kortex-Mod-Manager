#pragma once
#include "stdafx.h"
#include "KVariablesTable.h"
#include "KStaticVariablesTable.h"

// Class 'KDynamicVariablesTable' can expand variables whose values can change during runtime by themselves
// like date-time or say used memory.
class KDynamicVariablesTable: public KStaticVariablesTable
{
	private:
		using VariableFunctor = std::function<wxString(void)>;
		std::unordered_map<wxString, VariableFunctor> m_DynamicVariables;

	private:
		void NewVariable(const wxString& id, const VariableFunctor& functor);

	public:
		KDynamicVariablesTable();
		virtual ~KDynamicVariablesTable();

	public:
		virtual bool IsEmpty() const override;

		virtual bool HasVariable(const wxString& id) const override;
		virtual wxString GetVariable(const wxString& id) const override;
};
