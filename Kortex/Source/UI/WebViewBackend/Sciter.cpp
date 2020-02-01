#include "stdafx.h"
#include "Sciter.h"
#include <KxFramework/KxHTMLWindow.h>

namespace Kortex::UI::WebViewBackend
{
	void Sciter::OnLinkClicked(wxHtmlLinkEvent& event)
	{
		const wxHtmlLinkInfo& linkInfo = event.GetLinkInfo();
		if (SendEvent(IWebView::EvtNavigating, linkInfo.GetHref(), linkInfo.GetTarget()))
		{
			LoadURL(linkInfo.GetHref());
		}
	}
	bool Sciter::SendEvent(wxEventTypeTag<wxWebViewEvent> eventID, const wxString& url, const wxString& target)
	{
		wxWebViewEvent event(eventID, GetId(), url, target);
		event.Veto();
		m_EvtHandler.ProcessEvent(event);

		return event.IsAllowed();
	}

	Sciter::Sciter(wxWindow* parent, wxEvtHandler& evthandler, long style)
		:m_EvtHandler(evthandler)
	{
		if (WindowWrapper::Create(parent, KxID_NONE, wxDefaultPosition, wxDefaultSize, style))
		{
			Bind(wxEVT_HTML_LINK_CLICKED, &Sciter::OnLinkClicked, this);
		}
	}

	bool Sciter::LoadText(const wxString& text)
	{
		KxCallAtScopeExit atExit([this]()
		{
			SetFont(GetParent()->GetFont());
			SendEvent(IWebView::EvtLoaded);
		});

		auto FormatElement = [&](const wxString& html)
		{
			return KxString::Format(wxS("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset = utf-8\"/></head><body>%1</body></html>"), html);
		};
		return WindowWrapper::LoadHTML(FormatElement(KxHTMLWindow::ProcessPlainText(text)));
	}
	bool Sciter::LoadHTML(const wxString& html)
	{
		KxCallAtScopeExit atExit([this]()
		{
			SetFont(GetParent()->GetFont());
			SendEvent(IWebView::EvtLoaded);
		});
		return WindowWrapper::LoadHTML(html);
	}
	bool Sciter::LoadURL(const wxString& url)
	{
		KxCallAtScopeExit atExit([this, &url]()
		{
			SetFont(GetParent()->GetFont());

			SendEvent(IWebView::EvtNavigated, url);
			SendEvent(IWebView::EvtLoaded, url);
		});
		return WindowWrapper::LoadDocument(url);
	}
}
