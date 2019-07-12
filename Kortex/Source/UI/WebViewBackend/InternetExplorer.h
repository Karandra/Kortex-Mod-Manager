#pragma once
#include "stdafx.h"
#include "IWebView.h"
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

			KxColor GetBackgroundColor() const override
			{
				return m_WebView->GetBackgroundColour();
			}
			bool SetBackgroundColor(const KxColor& color)
			{
				return m_WebView->SetBackgroundColour(color);
			}

			KxColor GetForegroundColor() const override
			{
				return m_WebView->GetForegroundColour();
			}
			bool SetForegroundColor(const KxColor& color) override
			{
				return m_WebView->SetForegroundColour(color);
			}

		public:
			void Unload() override
			{
				m_WebView->LoadURL(wxWebViewDefaultURLStr);
			}
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
