#include "stdafx.h"
#include "Sciter.h"
#include <KxFramework/KxHTMLWindow.h>

namespace Kortex::UI::WebViewBackend
{
	void Sciter::OnLoaded(KxSciter::BehaviorEvent& event)
	{
		using namespace KxSciter;

		// Override primary font parent window font 
		SetFont(GetParent()->GetFont());

		// Make text selectable
		Element root = GetRootElement();
		root.Select("body", [](Element node)
		{
			node.SetStyleAttribute("behavior", "htmlarea");
			node.SetStyleAttribute("cursor", "text");

			return true;
		});

		// Remove YouTube embedded videos and replace them with a simple link
		root.Select("div.youtube_container", [](Element node)
		{
			// Get YouTube container
			Element iframe = node.GetFirstChild();
			wxString url = iframe.GetAttribute("src");
			iframe.Remove();

			// Convert links from embedded form to normal form
			wxRegEx regEx(wxS("https:\\/\\/www.youtube.com\\/(.+)\\/(.+)"));
			if (regEx.Matches(url) && regEx.GetMatch(url, 1) == wxS("embed"))
			{
				url = KxString::Format("https://www.youtube.com/watch?v=%1", regEx.GetMatch(url, 2));
			}

			// Create clickable link and attach it to the DOM
			Element link = Element::Create("a", url);
			link.SetAttribute("href", url);
			node.Append(link);

			return true;
		});

		// Make images fit into client area
		root.Select("img", [](Element node)
		{
			node.SetStyleAttribute("max-width", 100, SizeUnit::Percent);
			node.SetStyleAttribute("margin-left", "auto");
			node.SetStyleAttribute("margin-right", "auto");
			node.SetStyleAttribute("cursor", "arrow");

			return true;
		});

		// Process spoilers
		root.Select("div.bbc_spoiler_show", [](Element node)
		{
			node.SetStyleAttribute("display", "block");
			node.SetTagName("input");
			node.SetAttribute("type", "button");
			node.AttachEventHandler();

			return true;
		});
		root.Select("div.bbc_spoiler_content", [this](Element node)
		{
			node.SetStyleAttribute("border", "2dip dashed");
			node.SetStyleAttribute("border-color", GetForegroundColor().MakeDisabled());
			node.SetStyleAttribute("padding", KLC_HORIZONTAL_SPACING, SizeUnit::dip);
			node.SetStyleAttribute("margin", KLC_HORIZONTAL_SPACING, SizeUnit::dip);

			return true;
		});

		// Attach event handler to all link elements to prevent Sciter from opening them
		// and allowing us to show a dialog to the user to open them in default browser.
		root.Select("a", [](Element element)
		{
			element.AttachEventHandler();
			return true;
		});

		// Notify loaded
		SendEvent(IWebView::EvtLoaded);
	}
	void Sciter::OnHyperlink(KxSciter::BehaviorEvent& event)
	{
		SendEvent(IWebView::EvtNavigating, event.GetElement().GetAttribute("href"));
	}
	void Sciter::OnButton(KxSciter::BehaviorEvent& event)
	{
		if (KxSciter::Element element = event.GetElement().GetParent().SelectAny("div.bbc_spoiler_content"))
		{
			if (element.GetStyleAttribute("display") == "block")
			{
				element.SetStyleAttribute("display", "none");
				event.GetElement().SetText("Show");
			}
			else
			{
				element.SetStyleAttribute("display", "block");
				event.GetElement().SetText("Hide");
			}
		}
	}

	bool Sciter::SendEvent(wxEventTypeTag<wxWebViewEvent> eventID, const wxString& url, const wxString& target)
	{
		wxWebViewEvent event(eventID, GetId(), url, target);
		event.Veto();
		m_EvtHandler.ProcessEvent(event);

		return event.IsAllowed();
	}

	Sciter::Sciter(wxWindow* parent, wxEvtHandler& evthandler, long style)
		:m_EvtHandler(evthandler)
	{
		if (WindowWrapper::Create(parent, KxID_NONE, wxDefaultPosition, wxDefaultSize, style))
		{
			Bind(KxSciter::BehaviorEvent::EvtHyperlinkClick, &Sciter::OnHyperlink, this);
			Bind(KxSciter::BehaviorEvent::EvtDocumentReady, &Sciter::OnLoaded, this);
			Bind(KxSciter::BehaviorEvent::EvtButtonPress, &Sciter::OnButton, this);
		}
	}

	bool Sciter::LoadText(const wxString& text)
	{
		return WindowWrapper::LoadHTML(KxString::Format(wxS("<html><body><p>%1</p></body></html>"), KxHTMLWindow::ProcessPlainText(text)));
	}
	bool Sciter::LoadHTML(const wxString& html)
	{
		return WindowWrapper::LoadHTML(html);
	}
	bool Sciter::LoadURL(const wxString& url)
	{
		KxCallAtScopeExit atExit([this, &url]()
		{
			SendEvent(IWebView::EvtNavigated, url);
		});
		return WindowWrapper::LoadDocument(url);
	}
}
