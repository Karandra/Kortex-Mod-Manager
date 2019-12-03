#pragma once
#include "stdafx.h"
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxCallAtScopeExit.h>
#include "IWebView.h"

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

			KxColor GetBackgroundColor() const override
			{
				return KxHTMLWindow::GetBackgroundColour();
			}
			bool SetBackgroundColor(const KxColor& color) override
			{
				return KxHTMLWindow::SetBackgroundColour(color);
			}

			KxColor GetForegroundColor() const override
			{
				return KxHTMLWindow::GetForegroundColour();
			}
			bool SetForegroundColor(const KxColor& color) override
			{
				return KxHTMLWindow::SetForegroundColour(color);
			}

		public:
			void Unload() override
			{
				KxHTMLWindow::SetPage(wxEmptyString);
			}
			bool LoadText(const wxString& text) override;
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
