#pragma once
#include "stdafx.h"
#include "KLabeledValue.h"
class KxXMLNode;
class KxURI;

enum KAuxCharCode
{
	KAUX_CHAR_ARROW_LEFT = 0x2190,
	KAUX_CHAR_ARROW_UP,
	KAUX_CHAR_ARROW_RIGHT,
	KAUX_CHAR_ARROW_DOWN,
	KAUX_CHAR_ARROW_LEFT_RIGHT,
	KAUX_CHAR_ARROW_UP_DOWN,
};

class KAux
{
	public:
		// Return 's1' if it's not empty, otherwise return 's2'
		static wxString StrOr(const wxString& s1, const wxString& s2);
		static wxString StrOr(const wxString& s1, const wxString& s2, const wxString& s3);

		// Returns numerical architecture value: 32, 64.
		static wxString ArchitectureToNumber(bool is64Bit);

		// Returns string architecture value: "x86", "x64".
		static wxString ArchitectureToString(bool is64Bit);

		// Change lightness of the entire image using wxColour::ChangeLightness.
		static wxImage& ChangeLightness(wxImage& image, int alphaValue);

		// Get canonical ratio of given size such as '16:9' for 1920x1080 or 'unknown' if ratio can not be determined.
		static wxString GetResolutionRatio(const wxSize& resolution, const wxString& unknown = wxEmptyString);

		// Converts string to a bool value ("false" in any case is false and "0" or "0.0" is false, otherwise true).
		static bool StringToBool(const wxString& value, bool* isUnknown = nullptr);

		// Format date in Russian format (DD.MM.YYYY). Returns empty string if 't' is invalid.
		static wxString FormatDate(const wxDateTime& t);

		// Format time in Russian (or military) format (HH:MM:SS). Returns empty string if 't' is invalid.
		static wxString FormatTime(const wxDateTime& t);

		// Combined version of 'FormatDate' and 'FormatTime' with space as a separator. Returns empty string if 't' is invalid.
		static wxString FormatDateTime(const wxDateTime& t);

		// Formats date and time as 'DD-MM-YYYY HH-MM-SS'. Term 'FS' stands for 'File System' or 'File Safe'. Returns empty string if 't' is invalid.
		static wxString FormatDateTimeFS(const wxDateTime& t);

		// Checks if extension matches one of masks (* and ? symbols are supported). Masks in 'extensions' should be without dot and in lowercase.
		static bool IsFileExtensionMatches(const wxString& filePath, const KxStringVector& extensions);
		static bool IsSingleFileExtensionMatches(const wxString& filePath, const wxString& ext);

		// Shows a dialog that asks user to confirm opening the URI in default web-browser. Returns true if user has agreed.
		static bool AskOpenURL(const KxURI& uri, wxWindow* parent = nullptr);
		static bool AskOpenURL(const KLabeledValue::Vector& urlList, wxWindow* parent = nullptr);

		// Saves KLabeledValue::Vector into specified node, clearing the node first.
		// If KLabeledValue element has no label, attribute for the label will not be created.
		static void SaveLabeledValueArray(const KLabeledValue::Vector& array, KxXMLNode& arrayNode, const wxString& labelName = "Label");

		// Loads KLabeledValue::Vector from specified node.
		// Does NOT clears 'array'.
		static void LoadLabeledValueArray(KLabeledValue::Vector& array, const KxXMLNode& arrayNode, const wxString& labelName = "Label");

		// Saves KxStringVector into specified node, clearing the node first.
		static void SaveStringArray(const KxStringVector& array, KxXMLNode& arrayNode, const wxString& elementNodeName = "Entry");

		// Loads KxStringVector from specified node.
		// Does NOT clears 'array'.
		static void LoadStringArray(KxStringVector& array, const KxXMLNode& arrayNode);

		// Creates label enclosed in specified symbols.
		// MakeNoneLabel creates label with following format: <None>
		static wxString MakeNoneLabel()
		{
			return MakeBracketedLabel(KxID_NONE, '<', '>');
		}
		static wxString MakeBracketedLabel(wxStandardID id, const wxUniChar& cLeft = '<', const wxUniChar& cRight = '>');
		static wxString MakeBracketedLabel(int id, const wxUniChar& cLeft = '<', const wxUniChar& cRight = '>')
		{
			return MakeBracketedLabel((wxStandardID)id, cLeft, cRight);
		}
		static wxString MakeBracketedLabel(const wxString& text, const wxUniChar& cLeft = '<', const wxUniChar& cRight = '>');

		// Creates placeholder for KxHTMLWindow to be showed when actual content is unavailable.
		// Window is required if you want correct text color.
		static wxString MakeHTMLWindowPlaceholder(const wxString& text, const wxWindow* window = nullptr);

		// Extracts domain name from provided URL (without www. part if any).
		static wxString ExtractDomainName(const wxString& url);

		// Scales image to specified size maintaining aspect ratio
		static wxImage ScaleImageAspect(const wxImage& source, int width = -1, int height = -1);
		static wxBitmap ScaleImageAspect(const wxBitmap& source, int width = -1, int height = -1);

		// Checks is specified string contains chars forbidden by file system.
		static bool HasForbiddenFileNameChars(const wxString& string);

		// Removes all chars forbidden by file system from specified string.
		static wxString MakeSafeFileName(const wxString& string);

		// Creates filter for 'KxFileBrowseDialog' from array of extensions (without dot).
		static wxString MakeExtensionsFilter(const KxStringVector& extensions);

		// Get Unicode char by its code
		static wxUniChar GetUnicodeChar(int64_t code)
		{
			return wxUniChar(code);
		}
		static wxUniChar GetUnicodeChar(KAuxCharCode code)
		{
			return wxUniChar((int64_t)code);
		}

		template<class ContainerT, class StringT> static auto FindInStrings(const ContainerT& container, const StringT& string, bool caseSensetive = true)
		{
			if (caseSensetive)
			{
				return std::find(container.begin(), container.end(), string);
			}
			else
			{
				StringT lowerVariant = KxString::ToLower(string);
				return std::find_if(container.begin(), container.end(), [&lowerVariant](const StringT& v)
				{
					return KxString::ToLower(v) == lowerVariant;
				});
			}
		}
		template<class ContainerT, class StringT> static bool IsStringsContain(const ContainerT& container, const StringT& string, bool caseSensetive = true)
		{
			return FindInStrings(container, string) != container.end();
		}

		static bool SetSearchMask(wxString& storage, const wxString& newMask);
		static bool CheckSearchMask(const wxString& mask, const wxString& string);

		template<class T> static T ExpandVariablesInVector(const T& items)
		{
			T newItems;
			newItems.reserve(items.size());
			for (const auto& item: items)
			{
				newItems.emplace_back(KVarExp(item));
			}
			return newItems;
		}
		template<> static KLabeledValue::Vector ExpandVariablesInVector(const KLabeledValue::Vector& items)
		{
			KLabeledValue::Vector newItems;
			newItems.reserve(items.size());
			for (const auto& item: items)
			{
				newItems.emplace_back(KVarExp(item.GetValue()), KVarExp(item.GetLabelRaw()));
			}
			return newItems;
		}
};
