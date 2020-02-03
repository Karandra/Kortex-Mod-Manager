#pragma once
#include "stdafx.h"
#include <KxFramework/KxCallAtScopeExit.h>
#include <Kx/Sciter.hpp>
#include "IWebView.h"
class wxHtmlLinkEvent;

namespace Kortex::UI::WebViewBackend
{
	class Sciter: public KxSciter::WindowWrapper<wxControl>, public IWebView
	{
		private:
			wxEvtHandler& m_EvtHandler;

		private:
			void OnLoaded(KxSciter::BehaviorEvent& event);
			void OnHyperlink(KxSciter::BehaviorEvent& event);
			void OnButton(KxSciter::BehaviorEvent& event);

			bool SendEvent(wxEventTypeTag<wxWebViewEvent> eventID, const wxString& url = {}, const wxString& target = {});

		public:
			Sciter(wxWindow* parent, wxEvtHandler& evthandler, long style = 0);

		public:
			wxWindow* GetWindow() override
			{
				return this;
			}

			KxColor GetBackgroundColor() const override
			{
				return WindowWrapper::GetBackgroundColour();
			}
			bool SetBackgroundColor(const KxColor& color) override
			{
				return WindowWrapper::SetBackgroundColour(color);
			}

			KxColor GetForegroundColor() const override
			{
				return WindowWrapper::GetForegroundColour();
			}
			bool SetForegroundColor(const KxColor& color) override
			{
				return WindowWrapper::SetForegroundColour(color);
			}

		public:
			void Unload() override
			{
				WindowWrapper::ClearDocument();
			}
			bool LoadText(const wxString& text) override;
			bool LoadHTML(const wxString& html) override;
			bool LoadURL(const wxString& url) override;
	};
}
