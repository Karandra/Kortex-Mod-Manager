#pragma once
#include "stdafx.h"
#include <KxFramework/KxFormat.h>
class KGameInstance;

class KTranslation
{
	private:
		template<class Type> static wxString GetStringFallback(const Type& id)
		{
			return KxString::Format("$T(%1)", id);
		}

	public:
		static const KxTranslation& GetAppTranslation();

		template<class T> static wxString GetString(const KxTranslation& translation, const T& id)
		{
			bool isSuccess = false;
			wxString out = translation.GetString(id, &isSuccess);
			if (isSuccess)
			{
				return KVarExp(out);
			}
			return GetStringFallback(id);
		}
		template<class T> static wxString GetString(const KxTranslation& translation, const KGameInstance* instance, const T& id)
		{
			bool isSuccess = false;
			wxString out = translation.GetString(id, &isSuccess);
			if (isSuccess)
			{
				return KVarExp(instance, out);
			}
			return GetStringFallback(id);
		}

		template<class T, class... Args> static wxString FormatString(const KxTranslation& translation, const T& id, Args&&... args)
		{
			return KxString::Format(GetString(translation, id), std::forward<Args>(args)...);
		}
		template<class T, class... Args> static wxString FormatString(const KxTranslation& translation, const KGameInstance* instance, const T& id, Args&&... args)
		{
			return KxString::Format(GetString(translation, instance, id), std::forward<Args>(args)...);
		}
};

//////////////////////////////////////////////////////////////////////////
wxString KVarExp(const wxString& source);
wxString KVarExp(const KGameInstance* instance, const wxString& source);

//////////////////////////////////////////////////////////////////////////
template<class T> wxString KTr(const T& id)
{
	return KTranslation::GetString(KTranslation::GetAppTranslation(), id);
}
template<class T> wxString KTr(const KGameInstance* instance, const T& id)
{
	return KTranslation::GetString(KTranslation::GetAppTranslation(), instance, id);
}

template<class T, class... Args> wxString KTrf(const T& id, Args&&... args)
{
	return KTranslation::FormatString(KTranslation::GetAppTranslation(), id, std::forward<Args>(args)...);
}
template<class T, class... Args> wxString KTrf(const KGameInstance* instance, const T& id, Args&&... args)
{
	return KTranslation::FormatString(KTranslation::GetAppTranslation(), instance, id, std::forward<Args>(args)...);
}
