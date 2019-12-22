#include "stdafx.h"
#include "DynamicVariableTable.h"
#include "Utility/DateTime.h"

namespace Kortex
{
	bool DynamicVariableTable::IterateOverDynamic(const Visitor& visitor) const
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

	DynamicVariableTable::DynamicVariableTable()
	{
		DoSetDynamicVariable(wxS("CurrentDate"), []()
		{
			return Utility::DateTime::FormatDate(wxDateTime::Now());
		});
		DoSetDynamicVariable(wxS("CurrentTime"), []()
		{
			return Utility::DateTime::FormatTime(wxDateTime::Now());
		});
		DoSetDynamicVariable(wxS("CurrentDateTime"), []()
		{
			return Utility::DateTime::FormatDateTime(wxDateTime::Now());
		});
	}

	bool DynamicVariableTable::IsEmpty() const
	{
		return m_DynamicVariables.empty() && StaticVariableTable::IsEmpty();
	}

	bool DynamicVariableTable::HasVariable(const wxString& id) const
	{
		return m_DynamicVariables.count(id) || StaticVariableTable::HasVariable(id);
	}
	VariableValue DynamicVariableTable::GetVariable(const wxString& id) const
	{
		// Do not use our 'DynamicVariableTable::HasVariable' here. It's only for user
		// not for internal use.

		auto it = m_DynamicVariables.find(id);
		if (it != m_DynamicVariables.end())
		{
			return it->second();
		}
		return StaticVariableTable::GetVariable(id);
	}
	void DynamicVariableTable::SetVariable(const wxString& id, const VariableValue& value)
	{
		// Add new static variable only if this is not one of dynamic variables
		if (m_DynamicVariables.count(id) == 0)
		{
			StaticVariableTable::SetVariable(id, value);
		}
	}

	void DynamicVariableTable::Accept(const Visitor& visitor) const
	{
		if (IterateOverStatic(visitor))
		{
			IterateOverDynamic(visitor);
		}
	}
}
