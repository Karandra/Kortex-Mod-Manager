#include "stdafx.h"
#include "DateTime.h"

namespace Kortex::Utility::DateTime
{
	wxDateTime Now()
	{
		return wxDateTime::UNow();
	}

	bool IsLaterThanBy(const wxDateTime& date, const wxDateTime& compareTo, const wxTimeSpan& span)
	{
		return !date.IsEqualUpTo(compareTo, span);
	}
	bool IsNowLaterThanBy(const wxDateTime& date, const wxTimeSpan& span)
	{
		return IsLaterThanBy(Now(), date, span);
	}

	bool IsEarlierThanBy(const wxDateTime& date, const wxDateTime& compareTo, const wxTimeSpan& span)
	{
		return date.IsEqualUpTo(compareTo, span);
	}
	bool IsNowEarlierThanBy(const wxDateTime& date, const wxTimeSpan& span)
	{
		return IsEarlierThanBy(Now(), date, span);
	}
}
