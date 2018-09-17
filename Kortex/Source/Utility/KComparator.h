#pragma once
#include "stdafx.h"

namespace
{
	inline int CompareStringsAux(const wxString& v1, const wxString& v2, bool ignoreCase = true)
	{
		return ::CompareStringOrdinal(v1.wc_str(), v1.length(), v2.wc_str(), v2.length(), ignoreCase);
	}
}

namespace KComparator
{
	// ==
	template<class T> bool KEqual(const T& v1, const T& v2)
	{
		return v1 == v2;
	}
	inline bool KEqual(const wxString& v1, const wxString& v2, bool ignoreCase = true)
	{
		return CompareStringsAux(v1, v2, ignoreCase) == CSTR_EQUAL;
	}

	// <
	template<class T> bool KLess(const T& v1, const T& v2)
	{
		return v1 < v2;
	}
	inline bool KLess(const wxString& v1, const wxString& v2, bool ignoreCase = true)
	{
		return CompareStringsAux(v1, v2, ignoreCase) == CSTR_LESS_THAN;
	}

	// <=
	template<class T> bool KLessEqual(const T& v1, const T& v2)
	{
		return v1 <= v2;
	}
	inline bool KLessEqual(const wxString& v1, const wxString& v2, bool ignoreCase = true)
	{
		return CompareStringsAux(v1, v2, ignoreCase) == CSTR_LESS_THAN || CompareStringsAux(v1, v2, ignoreCase) == CSTR_EQUAL;
	}

	// >
	template<class T> bool KGreater(const T& v1, const T& v2)
	{
		return v1 > v2;
	}
	inline bool KGreater(const wxString& v1, const wxString& v2, bool ignoreCase = true)
	{
		return CompareStringsAux(v1, v2, ignoreCase) == CSTR_GREATER_THAN;
	}

	// >=
	template<class T> bool KGreaterEqual(const T& v1, const T& v2)
	{
		return v1 >= v2;
	}
	inline bool KGreaterEqual(const wxString& v1, const wxString& v2, bool ignoreCase = true)
	{
		return CompareStringsAux(v1, v2, ignoreCase) == CSTR_GREATER_THAN || CompareStringsAux(v1, v2, ignoreCase) == CSTR_EQUAL;
	}
}
