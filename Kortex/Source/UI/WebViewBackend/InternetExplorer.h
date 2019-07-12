#pragma once
#include "stdafx.h"
#include "UI/IWebView.h"
#include <wx/webview.h>
#include <KxFramework/KxCOM.h>
struct IWebBrowser2;
struct IHTMLDocument2;

namespace Kortex::UI::WebViewBackend
{
	class InternetExplorer: public IWebView
	{
		private:
			wxEvtHandler& m_EvtHandler;
			wxWebView* m_WebView = nullptr;
			bool m_IsEmpty = true;

		private:
			void DoLoadPage(const wxString& html);
			void DoLoadURL(const wxString& url);

			void OnNavigating(wxWebViewEvent& event);
			void OnNavigated(wxWebViewEvent& event);
			void OnLoaded(wxWebViewEvent& event);
			void OnError(wxWebViewEvent& event);

			IWebBrowser2& GetWebBrowser() const;
			KxCOMPtr<IHTMLDocument2> GetDocument2() const;
			bool ExecCommand(const wxString& command, const wxAny& arg = {});

		public:
			InternetExplorer(wxWindow* parent, wxEvtHandler& evthandler, long style = 0);

		public:
			wxWebView* GetWindow() override
			{
				return m_WebView;
			}

		public:
			bool LoadText(const wxString& text) override;
			bool LoadHTML(const wxString& html) override
			{
				DoLoadPage(html);
				return true;
			}
			bool LoadURL(const wxString& url) override
			{
				DoLoadURL(url);
				return true;
			}
	};
}
