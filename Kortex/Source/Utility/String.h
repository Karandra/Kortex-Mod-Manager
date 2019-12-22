#pragma once
#include "stdafx.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxFormat.h>
#include <KxFramework/KxStringUtility.h>

namespace Kortex::Utility
{
	enum CharCode: uint32_t
	{
		ArrowLeft = 0x2190,
		ArrowUp,
		ArrowRight,
		ArrowDown,
		ArrowLeftRight,
		ArrowUpDown,
	};
}

namespace Kortex::Utility::String
{
	using namespace KxUtility::String;

	// Converts string to a bool value
	std::optional<bool> ToBool(const wxString& value);

	// Return 's1' if it's not empty, otherwise return 's2'
	inline wxString StrOr(const wxString& s1, const wxString& s2)
	{
		if (!s1.IsEmpty())
		{
			return s1;
		}
		return s2;
	}
	
	// Returns string architecture value: "x86", "x64".
	inline wxString StrOr(const wxString& s1, const wxString& s2, const wxString& s3)
	{
		return StrOr(s1, StrOr(s2, s3));
	}

	// Get Unicode char by its code
	inline wxUniChar GetUnicodeChar(int64_t code)
	{
		return wxUniChar(code);
	}
	inline wxUniChar GetUnicodeChar(CharCode code)
	{
		return wxUniChar(static_cast<std::underlying_type_t<CharCode>>(code));
	}

	template<class TCollection, class TString>
	auto FindInStrings(const TCollection& collection, const TString& string, bool caseSensetive = true)
	{
		if (caseSensetive)
		{
			return std::find(collection.begin(), collection.end(), string);
		}
		else
		{
			TString lowerVariant = KxString::ToLower(string);
			return std::find_if(collection.begin(), collection.end(), [&lowerVariant](const TString& v)
			{
				return KxString::ToLower(v) == lowerVariant;
			});
		}
	}

	template<class TCollection, class TString>
	bool IsStringsContain(const TCollection& container, const TString& string, bool caseSensetive = true)
	{
		return FindInStrings(container, string) != container.end();
	}
}
