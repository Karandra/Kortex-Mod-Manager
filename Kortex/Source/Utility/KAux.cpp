#include "stdafx.h"
#include "Utility/KAux.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxString.h>
#include <KxFramework/KxLibrary.h>
#include <KxFramework/KxTaskDialog.h>

wxString KAux::StrOr(const wxString& s1, const wxString& s2)
{
	if (!s1.IsEmpty())
	{
		return s1;
	}
	return s2;
}
wxString KAux::StrOr(const wxString& s1, const wxString& s2, const wxString& s3)
{
	return StrOr(s1, StrOr(s2, s3));
}

wxString KAux::ArchitectureToNumber(bool is64Bit)
{
	return is64Bit ? "64" : "32";
}
wxString KAux::ArchitectureToString(bool is64Bit)
{
	return is64Bit ? "x64" : "x86";
}

wxImage& KAux::ChangeLightness(wxImage& image, int alphaValue)
{
	unsigned char* data = image.GetData();
	const size_t length = (size_t)image.GetWidth() * (size_t)image.GetHeight() * 3;
	for (size_t i = 0; i < length; i += 3)
	{
		unsigned char* r = &data[i];
		unsigned char* g = &data[i+1];
		unsigned char* b = &data[i+2];
		wxColour::ChangeLightness(r, g, b, alphaValue);
	}
	return image;
}
wxString KAux::GetResolutionRatio(const wxSize& resolution, const wxString& unknown)
{
	auto MakeRatio = [](float x, float y) constexpr -> int
	{
		return ((x / y) * 100) + 0.1;
	};

	static constexpr const int r3_2 = MakeRatio(3, 2);
	static constexpr const int r4_3 = MakeRatio(4, 3);
	static constexpr const int r5_3 = MakeRatio(5, 3);
	static constexpr const int r5_4 = MakeRatio(5, 4);
	static constexpr const int r11_4 = MakeRatio(11, 4);
	static constexpr const int r14_9 = MakeRatio(14, 9);
	static constexpr const int r16_9 = MakeRatio(16, 9);
	static constexpr const int r16_10 = MakeRatio(16, 10);
	static constexpr const int r17_8 = MakeRatio(17, 8) + 1; // Because 17:8 is 212 and I need 213
	static constexpr const int r18_9 = MakeRatio(18, 9);
	static constexpr const int r21_9 = MakeRatio(21, 9);

	switch (MakeRatio(resolution.GetWidth(), resolution.GetHeight()))
	{
		case r3_2:
		{
			return "3:2";
		}
		case r4_3:
		{
			return "4:3";
		}
		case r5_4:
		{
			return "5:4";
		}
		case r5_3:
		{
			return "5:3";
		}
		case r11_4:
		{
			return "11:4";
		}
		case r14_9:
		{
			return "14:9";
		}
		case r16_9:
		{
			return "16:9";
		}
		case r16_10:
		{
			return "16:10";
		}
		case r17_8:
		{
			return "17:8";
		}
		case r18_9:
		{
			return "18:9";
		}
		case r21_9:
		{
			return "21:9";
		}
	};
	return unknown;
}
bool KAux::StringToBool(const wxString& value, bool* isUnknown)
{
	return KxUtility::StringToBool(value, isUnknown);
}

wxString KAux::FormatDate(const wxDateTime& t)
{
	if (t.IsValid())
	{
		return t.Format("%d.%m.%Y");
	}
	return wxEmptyString;
}
wxString KAux::FormatTime(const wxDateTime& t)
{
	if (t.IsValid())
	{
		return t.FormatISOTime();
	}
	return wxEmptyString;
}
wxString KAux::FormatDateTime(const wxDateTime& t)
{
	if (t.IsValid())
	{
		return FormatDate(t) + ' ' + FormatTime(t);
	}
	return wxEmptyString;
}
wxString KAux::FormatDateTimeFS(const wxDateTime& t)
{
	if (t.IsValid())
	{
		return t.Format("%d-%m-%Y %H-%M-%S");
	}
	return wxEmptyString;
}

bool KAux::IsFileExtensionMatches(const wxString& filePath, const KxStringVector& extensions)
{
	wxString ext = KxString::ToLower(filePath.AfterLast('.'));
	for (const wxString& mask: extensions)
	{
		if (ext.Matches(mask))
		{
			return true;
		}
	}
	return false;
}
bool KAux::IsSingleFileExtensionMatches(const wxString& filePath, const wxString& ext)
{
	wxString fileExt = KxString::ToLower(filePath.AfterLast('.'));
	return fileExt.Matches(ext);
}
bool KAux::AskOpenURL(const wxString& url, wxWindow* parent)
{
	KxTaskDialog dialog(parent, KxID_NONE, KTr("Generic.OpenWebSiteDialog.Caption"), url, KxBTN_YES|KxBTN_NO);
	dialog.SetOptionEnabled(KxTD_SIZE_TO_CONTENT);
	if (dialog.ShowModal() == KxID_YES)
	{
		KxShell::Execute(parent, url, "open");
		return true;
	}
	return false;
}
bool KAux::AskOpenURL(const KLabeledValue::Vector& urlList, wxWindow* parent)
{
	KxTaskDialog dialog(parent, KxID_NONE, KTr("Generic.OpenWebSiteListDialog.Caption"), wxEmptyString, KxBTN_CANCEL);
	dialog.SetOptionEnabled(KxTD_SIZE_TO_CONTENT);
	dialog.SetOptionEnabled(KxTD_CMDLINKS_ENABLED);
	dialog.Bind(KxEVT_STDDIALOG_BUTTON, [parent, urlList](wxNotifyEvent& event)
	{
		if (event.GetId() != KxID_CANCEL)
		{
			KxShell::Execute(parent, urlList[event.GetId() - KxID_HIGHEST].GetValue(), "open");
		}
		event.Skip();
	});
	
	for (size_t i = 0; i < urlList.size(); i++)
	{
		const KLabeledValue& url = urlList[i];
		if (url.HasValue() && url.HasLabel())
		{
			dialog.AddButton(KxID_HIGHEST + i, wxString::Format("%s\n%s", url.GetLabel(), url.GetValue()));
		}
		else if (url.HasValue() && !url.HasLabel())
		{
			dialog.AddButton(KxID_HIGHEST + i, url.GetValue());
		}
	}
	return dialog.ShowModal() != KxID_CANCEL;
}

void KAux::SaveLabeledValueArray(const KLabeledValue::Vector& array, KxXMLNode& arrayNode, const wxString& labelName)
{
	arrayNode.ClearChildren();

	for (const KLabeledValue& value: array)
	{
		KxXMLNode elementNode = arrayNode.NewElement("Entry");

		elementNode.SetValue(value.GetValue());
		if (value.HasLabel())
		{
			elementNode.SetAttribute(labelName, value.GetLabel());
		}
	}
}
void KAux::LoadLabeledValueArray(KLabeledValue::Vector& array, const KxXMLNode& arrayNode, const wxString& labelName)
{
	for (KxXMLNode node = arrayNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
	{
		array.emplace_back(KLabeledValue(node.GetValue(), node.GetAttribute(labelName)));
	}
}
void KAux::SaveStringArray(const KxStringVector& array, KxXMLNode& arrayNode, const wxString& elementNodeName)
{
	arrayNode.ClearChildren();
	for (const wxString& value: array)
	{
		KxXMLNode elementNode = arrayNode.NewElement(elementNodeName);

		elementNode.SetValue(value);
	}
}
void KAux::LoadStringArray(KxStringVector& array, const KxXMLNode& arrayNode)
{
	for (KxXMLNode node = arrayNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
	{
		array.emplace_back(node.GetValue());
	}
}

wxString KAux::MakeBracketedLabel(wxStandardID id, const wxUniChar& cLeft, const wxUniChar& cRight)
{
	return MakeBracketedLabel(KTr(id), cLeft, cRight);
}
wxString KAux::MakeBracketedLabel(const wxString& text, const wxUniChar& cLeft, const wxUniChar& cRight)
{
	return wxString::Format("%c%s%c", (wxChar)cLeft, text, (wxChar)cRight);
}

wxString KAux::MakeHTMLWindowPlaceholder(const wxString& text, const wxWindow* window)
{
	wxString color("gray");
	if (window)
	{
		color = window->GetForegroundColour().MakeDisabled().GetAsString();
	}
	return wxString::Format("<br><br><font color=\"%s\"><div align=\"center\">%s</div></font>", color, text);
}
wxString KAux::ExtractDomainName(const wxString& url)
{
	wxString urlLower = KxString::ToLower(url);

	wxString regEx = wxString::FromUTF8Unchecked(u8R"((http:\/\/|https:\/\/|ftp:\/\/|www.)([\w_-]+(?:(?:\.[\w_-]+)+))([\w.,@?^=%&:\/~+#-]*[\w@?^=%&\/~+#-])*)");
	wxRegEx reURL(regEx, wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE|wxRE_NEWLINE);
	if (reURL.Matches(urlLower))
	{
		wxString name = reURL.GetMatch(urlLower, 2);
		if (name.StartsWith("www."))
		{
			return name.Mid(4);
		}
		return name;
	}
	return wxEmptyString;
}

wxImage KAux::ScaleImageAspect(const wxImage& source, int width, int height)
{
	double scale = 1;
	if (width != -1 && height != -1)
	{
		double scaleX = (double)width / source.GetWidth();
		double scaleY = (double)height / source.GetHeight();
		scale = std::min(scaleX, scaleY);
	}
	else if (width != -1)
	{
		scale = (double)width / source.GetWidth();
	}
	else if (height != -1)
	{
		scale = (double)height / source.GetHeight();
	}
	else
	{
		return wxNullImage;
	}
	return source.Scale(scale * source.GetWidth(), scale * source.GetHeight(), wxIMAGE_QUALITY_HIGH);
}
wxBitmap KAux::ScaleImageAspect(const wxBitmap& source, int width, int height)
{
	return wxBitmap(ScaleImageAspect(source.ConvertToImage(), width, height), 32);
}

bool KAux::HasForbiddenFileNameChars(const wxString& string)
{
	for (const auto& c: KxFile::GetForbiddenChars())
	{
		if (string.Contains(c))
		{
			return true;
		}
	}
	return false;
}
wxString KAux::MakeSafeFileName(const wxString& string)
{
	wxString out = string;
	for (auto& c: KxFile::GetForbiddenChars())
	{
		out.Replace(c, wxS("_"), true);
	}
	return out;
}
wxString KAux::MakeExtensionsFilter(const KxStringVector& extensions)
{
	return "*." + KxString::Join(extensions, ";*.");
}

bool KAux::SetSearchMask(wxString& storage, const wxString& newMask)
{
	if (storage != newMask)
	{
		if (!newMask.IsEmpty())
		{
			storage = '*' + KxString::ToLower(newMask) + '*';
		}
		else
		{
			storage.clear();
		}
		return true;
	}
	return false;
}
bool KAux::CheckSearchMask(const wxString& mask, const wxString& string)
{
	return mask.IsEmpty() || (mask.length() == 1 && *mask.begin() == wxS('*')) || KxString::ToLower(string).Matches(mask);
}
