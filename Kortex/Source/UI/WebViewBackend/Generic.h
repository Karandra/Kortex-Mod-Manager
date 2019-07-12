#pragma once
#include "stdafx.h"
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxCallAtScopeExit.h>
#include "UI/IWebView.h"

namespace Kortex::UI::WebViewBackend
{
	class Generic: public KxHTMLWindow, public IWebView
	{
		private:
			wxEvtHandler& m_EvtHandler;

		private:
			void OnLinkClicked(wxHtmlLinkEvent& event);
			bool SendEvent(wxEventTypeTag<wxWebViewEvent> eventID, const wxString& url = {}, const wxString& target = {});

		public:
			Generic(wxWindow* parent, wxEvtHandler& evthandler, long style = 0);

		public:
			wxWindow* GetWindow() override
			{
				return this;
			}

		public:
			bool LoadText(const wxString& text) override
			{
				KxCallAtScopeExit atExit([this]()
				{
					SendEvent(IWebView::EvtLoaded);
				});
				return DoSetValue(text);
			}
			bool LoadHTML(const wxString& html) override
			{
				KxCallAtScopeExit atExit([this]()
				{
					SendEvent(IWebView::EvtLoaded);
				});
				return KxHTMLWindow::SetPage(html);
			}
			bool LoadURL(const wxString& url) override
			{
				KxCallAtScopeExit atExit([this, &url]()
				{
					SendEvent(IWebView::EvtNavigated, url);
					SendEvent(IWebView::EvtLoaded, url);
				});
				return KxHTMLWindow::LoadPage(url);
			}
	};
}
