#pragma once
#include "stdafx.h"
#include <KxFramework/KxHTMLWindow.h>
#include "IWebView.h"

namespace Kortex::UI
{
	class WebView final: public wxEvtHandler, public IWebView
	{
		public:
			enum class Backend
			{
				Default = 0,
				Generic,
				InternetExplorer
			};

		private:
			wxEvtHandler m_EvtHandler;
			IWebView* m_Backend = nullptr;

		private:
			void OnNavigating(wxWebViewEvent& event);
			void OnNavigated(wxWebViewEvent& event);
			void OnLoaded(wxWebViewEvent& event);
			void OnError(wxWebViewEvent& event);
			void OnBackendDestroyed(wxWindowDestroyEvent& event);

		public:
			WebView() = default;
			WebView(wxWindow* parent, Backend backend = Backend::Default, long style = 0)
			{
				Create(parent, backend, style);
			}
			bool Create(wxWindow* parent, Backend backend = Backend::Default, long style = 0);

		public:
			IWebView* GetBackend() const
			{
				return m_Backend;
			}
			wxWindow* GetWindow() override
			{
				return m_Backend ? m_Backend->GetWindow() : nullptr;
			}

		public:
			bool LoadText(const wxString& text) override
			{
				return m_Backend->LoadText(text);
			}
			bool LoadHTML(const wxString& html) override
			{
				return m_Backend->LoadHTML(html);
			}
			bool LoadURL(const wxString& url) override
			{
				return m_Backend->LoadURL(url);
			}
	};
}
