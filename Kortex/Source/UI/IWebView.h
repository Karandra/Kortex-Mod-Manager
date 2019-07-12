#pragma once
#include "stdafx.h"
#include <wx/webview.h>

namespace Kortex::UI
{
	class IWebView
	{
		public:
			inline static const wxEventTypeTag<wxWebViewEvent> EvtNavigating = wxEVT_WEBVIEW_NAVIGATING;
			inline static const wxEventTypeTag<wxWebViewEvent> EvtNavigated = wxEVT_WEBVIEW_NAVIGATED;
			inline static const wxEventTypeTag<wxWebViewEvent> EvtLoaded = wxEVT_WEBVIEW_LOADED;
			inline static const wxEventTypeTag<wxWebViewEvent> EvtError = wxEVT_WEBVIEW_ERROR;

		public:
			IWebView() = default;
			virtual ~IWebView() = default;

		public:
			virtual wxWindow* GetWindow() = 0;
			const wxWindow* GetWindow() const
			{
				return const_cast<IWebView*>(this)->GetWindow();
			}

			operator wxWindow*()
			{
				return GetWindow();
			}
			operator const wxWindow*() const
			{
				return GetWindow();
			}

		public:
			virtual bool LoadText(const wxString& text) = 0;
			virtual bool LoadHTML(const wxString& html) = 0;
			virtual bool LoadURL(const wxString& url) = 0;
	};
}
