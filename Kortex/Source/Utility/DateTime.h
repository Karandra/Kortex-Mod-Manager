#pragma once
#include "stdafx.h"

namespace Kortex::Utility::DateTime
{
	wxDateTime Now();

	bool IsLaterThanBy(const wxDateTime& date, const wxDateTime& compareTo, const wxTimeSpan& span);
	bool IsNowLaterThanBy(const wxDateTime& date, const wxTimeSpan& span);

	bool IsEarlierThanBy(const wxDateTime& date, const wxDateTime& compareTo, const wxTimeSpan& span);
	bool IsNowEarlierThanBy(const wxDateTime& date, const wxTimeSpan& span);
}
