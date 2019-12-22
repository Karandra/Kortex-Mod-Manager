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

	wxString FormatDate(const wxDateTime& dateTime)
	{
		if (dateTime.IsValid())
		{
			return dateTime.Format(wxS("%d.%m.%Y"));
		}
		return wxEmptyString;
	}
	wxString FormatTime(const wxDateTime& dateTime)
	{
		if (dateTime.IsValid())
		{
			return dateTime.FormatISOTime();
		}
		return wxEmptyString;
	}
	wxString FormatDateTime(const wxDateTime& dateTime, wxChar sep)
	{
		if (dateTime.IsValid())
		{
			return FormatDate(dateTime) + sep + FormatTime(dateTime);
		}
		return wxEmptyString;
	}
	wxString FormatDateTimeFS(const wxDateTime& dateTime)
	{
		if (dateTime.IsValid())
		{
			return dateTime.Format(wxS("%d-%m-%Y %H-%M-%S"));
		}
		return wxEmptyString;
	}
}
