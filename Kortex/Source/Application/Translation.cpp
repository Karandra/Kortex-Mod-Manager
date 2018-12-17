#include "stdafx.h"
#include "Translation.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>

namespace Kortex
{
	wxString GetVariable(const wxString& variable, const wxString& variableNamespace)
	{
		return IApplication::GetInstance()->ExpandVariables(Variables::WrapAsInline(variable, variableNamespace));
	}
	wxString GetVariable(const IGameInstance& instance, const wxString& variable, const wxString& variableNamespace)
	{
		return instance.ExpandVariables(Variables::WrapAsInline(variable, variableNamespace));
	}
	wxString ExpandVariables(const wxString& variables)
	{
		return IApplication::GetInstance()->ExpandVariables(variables);
	}
	wxString ExpandVariables(const IGameInstance& instance, const wxString& source)
	{
		return instance.ExpandVariables(source);
	}
}
