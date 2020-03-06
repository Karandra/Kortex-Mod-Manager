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

	wxString FormatDate(const wxDateTime& dateTime, FormatFlags formatFlags)
	{
		if (dateTime.IsValid())
		{
			SYSTEMTIME localTime = {};
			dateTime.GetAsMSWSysTime(&localTime);

			wchar_t formattedDateTime[1024] = {};
			const DWORD flags = DATE_AUTOLAYOUT|(formatFlags & FormatFlags::Long ? DATE_LONGDATE : DATE_SHORTDATE);
			if (::GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, flags, &localTime, nullptr, formattedDateTime, std::size(formattedDateTime), nullptr) != 0)
			{
				return formattedDateTime;
			}
		}
		return wxEmptyString;
	}
	wxString FormatTime(const wxDateTime& dateTime, FormatFlags formatFlags)
	{
		if (dateTime.IsValid())
		{
			DWORD flags = 0;
			if (!(formatFlags & FormatFlags::Long))
			{
				flags |= TIME_NOSECONDS;
			}
			if (formatFlags & FormatFlags::Force24Hours)
			{
				flags |= TIME_FORCE24HOURFORMAT;
			}
			if (formatFlags & FormatFlags::NoTimeMarker)
			{
				flags |= TIME_NOTIMEMARKER;
			}

			SYSTEMTIME localTime = {};
			dateTime.GetAsMSWSysTime(&localTime);

			wchar_t formattedDateTime[1024] = {};
			if (::GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, flags, &localTime, nullptr, formattedDateTime, std::size(formattedDateTime)) != 0)
			{
				return formattedDateTime;
			}
		}
		return wxEmptyString;
	}
	wxString FormatDateTime(const wxDateTime& dateTime, FormatFlags formatFlags, const wxString& sep)
	{
		if (dateTime.IsValid())
		{
			return FormatDate(dateTime, formatFlags) + sep + FormatTime(dateTime, formatFlags);
		}
		return wxEmptyString;
	}
	wxString FormatDateTimeFS(const wxDateTime& dateTime)
	{
		if (dateTime.IsValid())
		{
			return dateTime.Format(wxS("%Y-%m-%d %H-%M-%S"));
		}
		return wxEmptyString;
	}
}
