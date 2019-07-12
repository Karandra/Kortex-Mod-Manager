#include "stdafx.h"
#include "Generic.h"

namespace Kortex::UI::WebViewBackend
{
	void Generic::OnLinkClicked(wxHtmlLinkEvent& event)
	{
		const wxHtmlLinkInfo& linkInfo = event.GetLinkInfo();
		if (SendEvent(IWebView::EvtNavigating, linkInfo.GetHref(), linkInfo.GetTarget()))
		{
			LoadURL(linkInfo.GetHref());
		}
	}
	bool Generic::SendEvent(wxEventTypeTag<wxWebViewEvent> eventID, const wxString& url, const wxString& target)
	{
		wxWebViewEvent event(eventID, GetId(), url, target);
		event.Veto();
		m_EvtHandler.ProcessEvent(event);

		return event.IsAllowed();
	}

	Generic::Generic(wxWindow* parent, wxEvtHandler& evthandler, long style)
		:m_EvtHandler(evthandler)
	{
		if (KxHTMLWindow::Create(parent, KxID_NONE, wxEmptyString, style))
		{
			Bind(wxEVT_HTML_LINK_CLICKED, &Generic::OnLinkClicked, this);
			SetBackgroundColour(parent->GetBackgroundColour());
		}
	}
}
