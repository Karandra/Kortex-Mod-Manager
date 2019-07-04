#pragma once
#include "stdafx.h"
#include <KxFramework/KxCOM.h>
#include <mshtmhst.h>
struct IWebBrowser2;
struct IHTMLDocument2;

namespace Kortex::UI
{
	class WebView final
	{
		private:
			wxWebView* m_View = nullptr;
			bool m_IsEmpty = true;

		private:
			void DoLoadPage(const wxString& html);
			void DoLoadURL(const wxString& url);

			void AskOpenURL(const wxString& url);
			void OnNavigate(wxWebViewEvent& event);
			void OnNavigated(wxWebViewEvent& event);

			IWebBrowser2& GetWebBrowser() const;
			KxCOMPtr<IHTMLDocument2> GetDocument2() const;
			bool ExecCommand(const wxString& command, const wxAny& arg = {});

		public:
			WebView() = default;
			WebView(wxWindow* parent, long style = 0)
			{
				Create(parent, style);
			}
			bool Create(wxWindow* parent, long style = 0);

		public:
			operator wxWebView*() const
			{
				return m_View;
			}
			wxWebView* GetWindow() const
			{
				return m_View;
			}

			void LoadText(const wxString& text);
			void LoadURL(const wxString& url);
	};
}
