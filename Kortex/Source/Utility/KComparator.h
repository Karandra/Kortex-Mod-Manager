#pragma once
#include "stdafx.h"

namespace KComparator
{
	template<class T> static int KCompare(const T& v1, const T& v2)
	{
		if (v1 > v2)
		{
			return 1;
		}
		else if (v1 < v2)
		{
			return -1;
		}
		return 0;
	}
	template<> static int KCompare(const wxString& v1, const wxString& v2)
	{
		return v1.CmpNoCase(v2);
	}
}
