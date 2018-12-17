#pragma once
#include "stdafx.h"
#include "IApplication.h"
#include <KxFramework/KxTranslation.h>
#include <KxFramework/KxFormat.h>
#include <optional>

namespace Kortex
{
	class IGameInstance;
}

namespace Kortex::Translation
{
	using OpString = std::optional<wxString>;
}
namespace Kortex::Translation::Internal
{
	template<class T> wxString ConstructTranslationVar(T&& id)
	{
		return KxString::Format(wxS("$T(%1)"), id);
	}

	template<class T> OpString DoGetString(const KxTranslation& translation, T&& id)
	{
		bool isSuccess = false;
		wxString out = translation.GetString(id, &isSuccess);
		if (isSuccess)
		{
			return ExpandVariables(out);
		}
		return std::nullopt;
	}
	template<class T> OpString DoGetString(const KxTranslation& translation, const IGameInstance& instance, T&& id)
	{
		bool isSuccess = false;
		wxString out = translation.GetString(id, &isSuccess);
		if (isSuccess)
		{
			return ExpandVariables(instance, out);
		}
		return std::nullopt;
	}
}
namespace Kortex::Translation
{
	template<class T> wxString GetString(const KxTranslation& translation, T&& id)
	{
		auto value = Internal::DoGetString(translation, id);
		return value.has_value() ? value.value() : Internal::ConstructTranslationVar(id);
	}
	template<class T> wxString GetString(const KxTranslation& translation, const IGameInstance& instance, const T& id)
	{
		auto value = Internal::DoGetString(translation, instance, id);
		return value.has_value() ? value.value() : Internal::ConstructTranslationVar(id);
	}
	template<class T> wxString GetString(T&& id)
	{
		return GetString(IApplication::GetInstance()->GetTranslation(), id);
	}
	template<class T> wxString GetString(const IGameInstance& instance, const T& id)
	{
		return GetString(IApplication::GetInstance()->GetTranslation(), instance, id);
	}

	template<class T> OpString TryGetString(const KxTranslation& translation, T&& id)
	{
		return Internal::DoGetString(translation, id);
	}
	template<class T> OpString TryGetString(const KxTranslation& translation, const IGameInstance& instance, T&& id)
	{
		return Internal::DoGetString(translation, instance, id);
	}
	template<class T> OpString TryGetString(T&& id)
	{
		return TryGetString(IApplication::GetInstance()->GetTranslation(), id);
	}
	template<class T> OpString TryGetString(const IGameInstance& instance, T&& id)
	{
		return GetString(IApplication::GetInstance()->GetTranslation(), instance, id);
	}

	template<class T, class... Args> wxString FormatString(const KxTranslation& translation, const T& id, Args&&... arg)
	{
		return KxString::Format(GetString(translation, id), std::forward<Args>(arg)...);
	}
	template<class T, class... Args> wxString FormatString(const KxTranslation& translation, const IGameInstance& instance, const T& id, Args&&... arg)
	{
		return KxString::Format(GetString(translation, instance, id), std::forward<Args>(arg)...);
	}
	template<class T, class... Args> wxString FormatString(const T& id, Args&&... arg)
	{
		return KxString::Format(GetString(id), std::forward<Args>(arg)...);
	}
	template<class T, class... Args> wxString FormatString(const IGameInstance& instance, const T& id, Args&&... arg)
	{
		return KxString::Format(GetString(instance, id), std::forward<Args>(arg)...);
	}

	template<class T, class... Args> OpString TryFormatString(const KxTranslation& translation, const T& id, Args&&... arg)
	{
		auto value = TryGetString(translation, id);
		if (value.has_value())
		{
			value = KxString::Format(value.value(), std::forward<Args>(arg)...);
		}
		return value;
	}
	template<class T, class... Args> wxString TryFormatString(const KxTranslation& translation, const IGameInstance& instance, const T& id, Args&&... arg)
	{
		auto value = TryGetString(translation, instance, id);
		if (value.has_value())
		{
			value = KxString::Format(value.value(), std::forward<Args>(arg)...);
		}
		return value;
	}
	template<class T, class... Args> OpString TryFormatString(const T& id, Args&&... arg)
	{
		auto value = TryGetString(id);
		if (value.has_value())
		{
			value = KxString::Format(value.value(), std::forward<Args>(arg)...);
		}
		return value;
	}
	template<class T, class... Args> wxString TryFormatString(const IGameInstance& instance, const T& id, Args&&... arg)
	{
		auto value = TryGetString(instance, id);
		if (value.has_value())
		{
			value = KxString::Format(value.value(), std::forward<Args>(arg)...);
		}
		return value;
	}
}

namespace Kortex
{
	wxString GetVariable(const wxString& variable, const wxString& variableNamespace = wxEmptyString);
	wxString GetVariable(const IGameInstance& instance, const wxString& variable, const wxString& variableNamespace = wxEmptyString);
	wxString ExpandVariables(const wxString& variables);
	wxString ExpandVariables(const IGameInstance& instance, const wxString& variables);

	template<class T> wxString Translate(T&& id)
	{
		return Translation::GetString(id);
	}
	template<class T> wxString Translate(const IGameInstance& instance, T&& id)
	{
		return Translation::GetString(instance, id);
	}

	template<class T, class... Args> wxString FormatTranslate(T&& id, Args&&... arg)
	{
		return Translation::FormatString(id, std::forward<Args>(arg)...);
	}
	template<class T, class... Args> wxString FormatTranslate(const IGameInstance& instance, T&& id, Args&&... arg)
	{
		return Translation::FormatString(instance, id, std::forward<Args>(arg)...);
	}
}

//////////////////////////////////////////////////////////////////////////
#define KVarExp Kortex::ExpandVariables
#define KTr Kortex::Translate
#define KTrf Kortex::FormatTranslate
