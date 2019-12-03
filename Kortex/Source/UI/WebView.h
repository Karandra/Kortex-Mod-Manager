#pragma once
#include "stdafx.h"
#include <KxFramework/KxHTMLWindow.h>
#include "WebViewBackend/IWebView.h"

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

			KxColor GetBackgroundColor() const override
			{
				return m_Backend->GetBackgroundColor();
			}
			bool SetBackgroundColor(const KxColor& color) override
			{
				return m_Backend->SetBackgroundColor(color);
			}

			KxColor GetForegroundColor() const override
			{
				return m_Backend->GetForegroundColor();
			}
			bool SetForegroundColor(const KxColor& color) override
			{
				return m_Backend->SetForegroundColor(color);
			}

		public:
			void Unload() override
			{
				m_Backend->Unload();
			}
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
