#pragma once
#include "stdafx.h"
#include "KVariablesTable.h"
#include "KStaticVariablesTable.h"

// Class 'KDynamicVariablesTable' can expand variables whose values can change during runtime by themselves
// like date-time or say used memory.
class KDynamicVariablesTable: public KStaticVariablesTable
{
	public:
		using VariableFunctor = std::function<KIVariableValue(void)>;

	private:
		std::unordered_map<wxString, VariableFunctor> m_DynamicVariables;

	protected:
		void DoSetDynamicVariable(const wxString& id, const VariableFunctor& functor);
		bool IterateOverDynamyc(const Visitor& visitor) const;

	public:
		KDynamicVariablesTable();
		virtual ~KDynamicVariablesTable();

	public:
		virtual bool IsEmpty() const override;

		virtual bool HasVariable(const wxString& id) const override;
		virtual KIVariableValue GetVariable(const wxString& id) const override;
		virtual void SetVariable(const wxString& id, const KIVariableValue& value) override;
		void SetDynamicVariable(const wxString& id, const VariableFunctor& functor);

		virtual void Accept(const Visitor& visitor) const override;
};
