#pragma once
#include "stdafx.h"
#include "Application/IVariableTable.h"
#include "StaticVariableTable.h"

// Class 'DynamicVariableTable' can expand variables whose values can change during runtime by themselves
// like date-time or say used memory.

namespace Kortex
{
	class DynamicVariableTable: public StaticVariableTable
	{
		public:
			using VariableFunctor = std::function<VariableValue(void)>;

		private:
			std::unordered_map<wxString, VariableFunctor> m_DynamicVariables;

		protected:
			template<class T> void DoSetDynamicVariable(const wxString& id, T&& functor)
			{
				// Add new dynamic variable only if this is not one of static variables
				if (m_StaticVariables.count(id) == 0)
				{
					m_DynamicVariables.insert_or_assign(id, functor);
				}
			}
			bool IterateOverDynamic(const Visitor& visitor) const;

		public:
			DynamicVariableTable();

		public:
			bool IsEmpty() const override;

			bool HasVariable(const wxString& id) const override;
			VariableValue GetVariable(const wxString& id) const override;
			void SetVariable(const wxString& id, const VariableValue& value) override;

			void SetDynamicVariable(const wxString& id, const VariableFunctor& functor)
			{
				DoSetDynamicVariable(id, functor);
			}
			void SetDynamicVariable(const wxString& id, VariableFunctor&& functor)
			{
				DoSetDynamicVariable(id, std::move(functor));
			}

			void Accept(const Visitor& visitor) const override;
	};
}
