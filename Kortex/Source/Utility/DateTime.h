#pragma once
#include "stdafx.h"
#include "EnumClassOperations.h"

namespace Kortex::Utility::DateTime
{
	enum class FormatFlags
	{
		None = 0,
		Long = 1 << 0,
		Force24Hours = 1 << 2,
		NoTimeMarker = 1 << 3
	};
}

namespace Kortex::Utility::DateTime
{
	wxDateTime Now();

	bool IsLaterThanBy(const wxDateTime& date, const wxDateTime& compareTo, const wxTimeSpan& span);
	bool IsNowLaterThanBy(const wxDateTime& date, const wxTimeSpan& span);

	bool IsEarlierThanBy(const wxDateTime& date, const wxDateTime& compareTo, const wxTimeSpan& span);
	bool IsNowEarlierThanBy(const wxDateTime& date, const wxTimeSpan& span);

	// Format date in Russian format (DD.MM.YYYY). Returns empty string if 'dateTime' is invalid.
	wxString FormatDate(const wxDateTime& dateTime, FormatFlags formatFlags = FormatFlags::None);

	// Format time in military format (HH:MM:SS). Returns empty string if 'dateTime' is invalid.
	wxString FormatTime(const wxDateTime& dateTime, FormatFlags formatFlags = FormatFlags::None);

	// Combined version of 'FormatDate' and 'FormatTime' with a separator (space by default). Returns empty string if 'dateTime' is invalid.
	wxString FormatDateTime(const wxDateTime& dateTime, FormatFlags formatFlags = FormatFlags::None, const wxString& sep = wxS(' '));

	// Formats date and time as 'DD-MM-YYYY HH-MM-SS'. Term 'FS' stands for 'File System' or 'File Safe'. Returns empty string if 'dateTime' is invalid.
	wxString FormatDateTimeFS(const wxDateTime& dateTime);
}

namespace KxEnumClassOperations
{
	KxImplementEnum(Kortex::Utility::DateTime::FormatFlags);
}
