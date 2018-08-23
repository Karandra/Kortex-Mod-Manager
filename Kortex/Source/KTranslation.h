#pragma once
#include "stdafx.h"
#include <KxFramework/KxFormat.h>
class KProfile;

namespace
{
	template<class Type> wxString T_Fallback(const Type& id)
	{
		if constexpr(std::is_integral<Type>::value || std::is_enum<Type>::value)
		{
			return wxString::Format("$T(%d)", id);
		}
		else
		{
			return wxString::Format("$T(%s)", id);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
wxString V(const wxString& source);
wxString V(KProfile* profile, const wxString& source);

//////////////////////////////////////////////////////////////////////////
template<class Type> wxString T(const Type& id)
{
	bool isSuccess = false;
	wxString out = KxTranslation::GetString(id, &isSuccess);
	if (isSuccess)
	{
		return V(out);
	}
	else
	{
		return T_Fallback(id);
	}
}
template<class Type> wxString T(KProfile* profile, const Type& id)
{
	bool isSuccess = false;
	wxString out = KxTranslation::GetString(id, &isSuccess);
	if (isSuccess)
	{
		return V(profile, out);
	}
	else
	{
		return T_Fallback(id);
	}
}

template<class Type> KxFormat TF(const Type& id)
{
	return T(id);
}
template<class Type> KxFormat TF(KProfile* profile, const Type& id)
{
	return T(profile, id);
}
