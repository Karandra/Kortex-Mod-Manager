#include "stdafx.h"
#include "VariableValue.h"
#include "Application/IApplication.h"

namespace Kortex
{
	wxString VariableValue::Expand() const
	{
		return IApplication::GetInstance()->ExpandVariables(m_Value);
	}
}
