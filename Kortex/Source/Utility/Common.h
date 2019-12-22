#pragma once
#include "stdafx.h"
class KxURI;

namespace Kortex::Utility
{
	bool CopyTextToClipboard(const wxString& text);

	// Returns numerical architecture value: 32, 64.
	inline wxString ArchitectureToNumber(bool is64Bit)
	{
		return is64Bit ? wxS("64") : wxS("32");
	}
	
	// Returns string architecture value: x86, x64.
	inline wxString ArchitectureToString(bool is64Bit)
	{
		return is64Bit ? wxS("x64") : wxS("x86");
	}

	// Canonical ratio of a given size such as '16:9' for 1920x1080 or an empty string if the ratio can not be determined.
	wxString GetResolutionRatio(const wxSize& resolution);

	// Checks if extension matches one of masks (* and ? symbols are supported). Masks in 'extensions' should be without dot and in lowercase.
	bool FileExtensionMatches(const wxString& filePath, const KxStringVector& extensions);
	bool SingleFileExtensionMatches(const wxString& filePath, const wxString& ext);

	// Creates filter for 'KxFileBrowseDialog' from array of extensions (without dot).
	wxString MakeExtensionsFilter(const KxStringVector& extensions);

	// Checks is specified string contains chars forbidden by the file system.
	bool HasForbiddenFileNameChars(const wxString& string);

	// Removes all chars forbidden by file system from specified string.
	wxString MakeSafeFileName(const wxString& string);

	// Creates label enclosed in specified symbols.
	// MakeNoneLabel creates label with following format: <None>
	wxString MakeBracketedLabel(wxStandardID id, const wxUniChar& cLeft = wxS('<'), const wxUniChar& cRight = wxS('>'));
	wxString MakeBracketedLabel(KxStandardID id, const wxUniChar& cLeft = wxS('<'), const wxUniChar& cRight = wxS('>'));
	wxString MakeBracketedLabel(const wxString& text, const wxUniChar& cLeft = wxS('<'), const wxUniChar& cRight = wxS('>'));
	inline wxString MakeNoneLabel()
	{
		return MakeBracketedLabel(KxID_NONE, wxS('<'), wxS('>'));
	}

	template<class TItems>
	TItems ExpandVariablesInVector(const TItems& items)
	{
		TItems newItems;
		newItems.reserve(items.size());
		for (const auto& item: items)
		{
			newItems.emplace_back(KVarExp(item));
		}
		return newItems;
	}
}
