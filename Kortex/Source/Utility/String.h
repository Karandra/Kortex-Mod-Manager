#pragma once
#include "stdafx.h"

namespace Kortex::Utility::String
{
	using TStdWxString = typename std::basic_string<wxChar, std::char_traits<wxChar>, std::allocator<wxChar>>;
	using TStdWxStringView = typename std::basic_string_view<wxChar, std::char_traits<wxChar>>;
}

namespace Kortex::Utility::String
{
	template<class... Args> wxString Concat(Args&&... arg)
	{
		return (wxString(arg) + ...);
	}
	template<class... Args> wxString ConcatWithSeparator(const wxString& sep, Args&&... arg)
	{
		return ((wxString(arg) + sep) + ...);
	}

	template<class TFunctor> void SplitBySeparator(const wxString& string, const wxString& sep, const TFunctor& func)
	{
		// This ugly construction is faster than 'KxString::Split', so using it.
		size_t pos = 0;
		size_t separatorPos = string.find(sep);
		if (separatorPos == wxString::npos)
		{
			separatorPos = string.length();
		}

		while (pos < string.length() && separatorPos <= string.length())
		{
			const TStdWxStringView stringPiece(string.wc_str() + pos, separatorPos - pos);
			if (func(stringPiece))
			{
				return;
			}

			pos += stringPiece.length() + sep.length();
			separatorPos = string.find(sep, pos);

			// No separator found, but this is not the last element
			if (separatorPos == wxString::npos && pos < string.length())
			{
				separatorPos = string.length();
			}
		}
	}
	template<class TFunctor> void SplitByLength(const wxString& string, size_t length, const TFunctor& func)
	{
		const stringLength = string.length();
		for (size_t i = 0; i < stringLength; i += length)
		{
			const TStdWxStringView stringPiece(string.wc_str() + i, std::min(length, stringLength - i));
			if (func(stringPiece))
			{
				return;
			}
		}
	}
}
