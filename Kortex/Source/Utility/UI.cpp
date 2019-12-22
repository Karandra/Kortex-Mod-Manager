#include "stdafx.h"
#include "UI.h"
#include "Application/Resources/IImageProvider.h"
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxURI.h>

namespace Kortex::Utility::UI
{
	KxAuiToolBarItem* CreateToolBarButton(KxAuiToolBar* toolBar,
										  const wxString& label,
										  const ResourceID& imageID,
										  wxItemKind kind,
										  int index
	)
	{
		wxBitmap bitmap = ImageProvider::GetBitmap(imageID);
		KxAuiToolBarItem* button = toolBar->AddTool(label, bitmap, kind);
		if (!toolBar->HasFlag(wxAUI_TB_TEXT))
		{
			button->SetShortHelp(label);
		}

		return button;
	}

	bool AskOpenURL(const KxURI& uri, wxWindow* parent)
	{
		KxTaskDialog dialog(parent, KxID_NONE, KTr("Generic.OpenWebSiteDialog.Caption"), uri.BuildUnescapedURI(), KxBTN_YES|KxBTN_NO);
		dialog.SetOptionEnabled(KxTD_SIZE_TO_CONTENT);
		if (dialog.ShowModal() == KxID_YES)
		{
			return KxShell::Execute(parent, uri.BuildUnescapedURI());
		}
		return false;
	}
	bool AskOpenURL(const LabeledValue::Vector& urlList, wxWindow* parent)
	{
		KxTaskDialog dialog(parent, KxID_NONE, KTr("Generic.OpenWebSiteListDialog.Caption"), wxEmptyString, KxBTN_CANCEL);
		dialog.SetOptionEnabled(KxTD_SIZE_TO_CONTENT);
		dialog.SetOptionEnabled(KxTD_CMDLINKS_ENABLED);
		dialog.Bind(KxEVT_STDDIALOG_BUTTON, [parent, urlList](wxNotifyEvent& event)
		{
			if (event.GetId() != KxID_CANCEL)
			{
				KxShell::Execute(parent, urlList[event.GetId() - KxID_HIGHEST].GetValue());
			}
			event.Skip();
		});

		for (size_t i = 0; i < urlList.size(); i++)
		{
			const Utility::LabeledValue& url = urlList[i];
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

	wxString MakeHTMLWindowPlaceholder(const wxString& text, const wxWindow* window)
	{
		wxString color = window ? window->GetForegroundColour().MakeDisabled().GetAsString() : wxS("gray");
		return wxString::Format(wxS("<br><br><font color=\"%s\"><div align=\"center\">%s</div></font>"), color, text);
	}

	bool SetSearchMask(wxString& storage, const wxString& newMask)
	{
		if (storage != newMask)
		{
			if (!newMask.IsEmpty())
			{
				storage = wxS('*') + KxString::ToLower(newMask) + wxS('*');
			}
			else
			{
				storage.clear();
			}
			return true;
		}
		return false;
	}
	bool CheckSearchMask(const wxString& mask, const wxString& string)
	{
		return mask.IsEmpty() || (mask.length() == 1 && *mask.begin() == wxS('*')) || KxString::ToLower(string).Matches(mask);
	}
}
