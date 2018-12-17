#pragma once
#include "stdafx.h"
#include "Application/IVariableTable.h"

// This is static variables table. It can return values that explicitly saved there
// with only exception of 'Expand' function which can get some variables dynamically.
// If you need more flexible behavior take a look at 'DynamicVariableTable'.

namespace Kortex
{
	class StaticVariableTable: public IVariableTable
	{
		protected:
			std::unordered_map<wxString, VariableValue> m_StaticVariables;

		protected:
			bool IterateOverStatic(const Visitor& visitor) const;

		public:
			StaticVariableTable(size_t initialCount = 0)
			{
				m_StaticVariables.reserve(initialCount);
			}

		public:
			bool IsEmpty() const;

			bool HasVariable(const wxString& id) const override;
			VariableValue GetVariable(const wxString& id) const override;
			void SetVariable(const wxString& id, const VariableValue& value) override;

			void Accept(const Visitor& visitor) const override
			{
				IterateOverStatic(visitor);
			}
	};
}
