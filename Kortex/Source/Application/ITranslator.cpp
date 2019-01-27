#include "stdafx.h"
#include "ITranslator.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>

namespace Kortex
{
	const ITranslator& ITranslator::GetAppTranslator()
	{
		return IApplication::GetInstance()->GetTranslator();
	}

	wxString ITranslator::GetVariable(const wxString& variable, const wxString& variableNamespace)
	{
		return IApplication::GetInstance()->ExpandVariables(Variables::WrapAsInline(variable, variableNamespace));
	}
	wxString ITranslator::GetVariable(const IGameInstance& instance, const wxString& variable, const wxString& variableNamespace)
	{
		return instance.ExpandVariables(Variables::WrapAsInline(variable, variableNamespace));
	}
	wxString ITranslator::ExpandVariables(const wxString& variables)
	{
		return IApplication::GetInstance()->ExpandVariables(variables);
	}
	wxString ITranslator::ExpandVariables(const IGameInstance& instance, const wxString& source)
	{
		return instance.ExpandVariables(source);
	}
}
