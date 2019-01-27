#pragma once
#include "stdafx.h"
#include "ITranslator.h"

namespace Kortex
{
	class IGameInstance;

	template<class T> wxString Translate(T&& id)
	{
		return ITranslator::GetAppTranslator().GetString(id);
	}
	template<class T> wxString Translate(const IGameInstance& instance, T&& id)
	{
		return ITranslator::GetAppTranslator().GetString(instance, id);
	}

	template<class T, class... Args> wxString FormatTranslate(T&& id, Args&&... arg)
	{
		return ITranslator::GetAppTranslator().FormatString(id, std::forward<Args>(arg)...);
	}
	template<class T, class... Args> wxString FormatTranslate(const IGameInstance& instance, T&& id, Args&&... arg)
	{
		return ITranslator::GetAppTranslator().FormatString(instance, id, std::forward<Args>(arg)...);
	}
}

#define KVarExp Kortex::ITranslator::ExpandVariables
#define KTr Kortex::Translate
#define KTrf Kortex::FormatTranslate
