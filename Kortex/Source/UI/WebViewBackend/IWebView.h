#pragma once
#include "stdafx.h"
#include <KxFramework/KxEvent.h>
#include <wx/webview.h>

namespace Kortex::UI
{
	class IWebView
	{
		public:
			KxEVENT_MEMBER_AS(wxWebViewEvent, Navigating, wxEVT_WEBVIEW_NAVIGATING);
			KxEVENT_MEMBER_AS(wxWebViewEvent, Navigated, wxEVT_WEBVIEW_NAVIGATED);
			KxEVENT_MEMBER_AS(wxWebViewEvent, Loaded, wxEVT_WEBVIEW_LOADED);
			KxEVENT_MEMBER_AS(wxWebViewEvent, Error, wxEVT_WEBVIEW_ERROR);

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

			virtual KxColor GetBackgroundColor() const = 0;
			virtual bool SetBackgroundColor(const KxColor& color) = 0;

			virtual KxColor GetForegroundColor() const = 0;
			virtual bool SetForegroundColor(const KxColor& color) = 0;

		public:
			virtual void Unload() = 0;
			virtual bool LoadText(const wxString& text) = 0;
			virtual bool LoadHTML(const wxString& html) = 0;
			virtual bool LoadURL(const wxString& url) = 0;
	};
}
