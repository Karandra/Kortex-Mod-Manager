#pragma once
#include "stdafx.h"
#include "KVariablesTable.h"

// KDynamicVariablesTable can expand variables whose values can change during runtime by themselves
// like date-time or say used memory.
class KDynamicVariablesTable: public KVariablesTable
{
	private:
		typedef std::function<wxString(void)> EntryType;
		std::unordered_map<wxString, EntryType> m_DynamicVariables;

	private:
		void NewVariable(const wxString& id, const EntryType& functor);

	public:
		KDynamicVariablesTable();
		virtual ~KDynamicVariablesTable();

	public:
		virtual bool IsEmpty() const override;

		virtual bool HasVariable(const wxString& id) const override;
		virtual wxString GetVariable(const wxString& id) const override;
};
