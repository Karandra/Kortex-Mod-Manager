#pragma once
#include "stdafx.h"
#include <KxFramework/KxFormat.h>
class KGameInstance;

namespace KTranslation
{
	const KxTranslation& GetTranslation();

	template<class Type> wxString T_Fallback(const Type& id)
	{
		return KxFormat("$T(%1)").arg(id);
	}
}

//////////////////////////////////////////////////////////////////////////
wxString V(const wxString& source);
wxString V(KGameInstance* profile, const wxString& source);

//////////////////////////////////////////////////////////////////////////
template<class Type> wxString T(const Type& id)
{
	bool isSuccess = false;
	wxString out = KTranslation::GetTranslation().GetString(id, &isSuccess);
	if (isSuccess)
	{
		return V(out);
	}
	return KTranslation::T_Fallback(id);
}
template<class Type> wxString T(KGameInstance* profile, const Type& id)
{
	bool isSuccess = false;
	wxString out = KTranslation::GetTranslation().GetString(id, &isSuccess);
	if (isSuccess)
	{
		return V(profile, out);
	}
	return KTranslation::T_Fallback(id);
}

template<class Type> KxFormat TF(const Type& id)
{
	return T(id);
}
template<class Type> KxFormat TF(KGameInstance* profile, const Type& id)
{
	return T(profile, id);
}
