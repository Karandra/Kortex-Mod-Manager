#include "stdafx.h"
#include "WebView.h"
#include "WebViewBackend/Generic.h"
#include "WebViewBackend/InternetExplorer.h"
#include "Utility/KAux.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxURI.h>

namespace Kortex::UI
{
	void WebView::OnNavigating(wxWebViewEvent& event)
	{
		// If event wasn't processed by outside code ask to open the url in external browser
		if (!ProcessEvent(event))
		{
			KAux::AskOpenURL(event.GetURL(), GetWindow());

			// And disallow to open this url in the web view
			event.Veto();
		}
	}
	void WebView::OnNavigated(wxWebViewEvent& event)
	{
		ProcessEvent(event);
	}
	void WebView::OnLoaded(wxWebViewEvent& event)
	{
		ProcessEvent(event);
	}
	void WebView::OnError(wxWebViewEvent& event)
	{
		ProcessEvent(event);
	}
	void WebView::OnBackendDestroyed(wxWindowDestroyEvent& event)
	{
		m_Backend = nullptr;
	}

	bool WebView::Create(wxWindow* parent, Backend backend, long style)
	{
		if (parent)
		{
			// Bind events
			m_EvtHandler.Bind(IWebView::EvtNavigating, &WebView::OnNavigating, this);
			m_EvtHandler.Bind(IWebView::EvtNavigated, &WebView::OnNavigated, this);
			m_EvtHandler.Bind(IWebView::EvtLoaded, &WebView::OnLoaded, this);
			m_EvtHandler.Bind(IWebView::EvtError, &WebView::OnError, this);

			// Create backend
			switch (backend)
			{
				case Backend::Default:
				case Backend::Generic:
				{
					m_Backend = new WebViewBackend::Generic(parent, m_EvtHandler, style);
					break;
				}
				case Backend::InternetExplorer:
				{
					m_Backend = new WebViewBackend::InternetExplorer(parent, m_EvtHandler, style);
					break;
				}
			};
			
			if (m_Backend)
			{
				m_Backend->GetWindow()->Bind(wxEVT_DESTROY, &WebView::OnBackendDestroyed, this);
				return true;
			}
		}
		return false;
	}
}
