#include "stdafx.h"
#include "Sciter.h"
#include <KxFramework/KxHTMLWindow.h>

namespace Kortex::UI::WebViewBackend
{
	void Sciter::OnLoaded(KxSciter::BehaviorEvent& event)
	{
		using namespace KxSciter;

		// Override primary document font to parent window font (but a bit bigger)
		SetFont(GetParent()->GetFont().Scaled(1.1f));

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
		root.Select("img", [](Element image)
		{
			image.SetStyleAttribute("max-width", 100, SizeUnit::Percent);
			image.SetStyleAttribute("margin-left", "auto");
			image.SetStyleAttribute("margin-right", "auto");
			image.SetStyleAttribute("cursor", "arrow");

			return true;
		});

		// Convert spoiler buttons to Sciter button control
		root.Select("div.bbc_spoiler_show", [&](Element button)
		{
			button.SetTagName("button");
			button.SetStyleAttribute("display", "flex");
			button.SetAttribute("type", "button");
			button.SetText(KTr(KxID_OPEN));

			button.AttachEventHandler(m_SpoilerButtonHandler);
			return true;
		});
		root.Select("div.bbc_spoiler_content", [this](Element content)
		{
			content.SetStyleAttribute("display", "none");
			content.SetStyleAttribute("border", "2dip dashed");
			content.SetStyleAttribute("border-color", GetForegroundColor().MakeDisabled());
			content.SetStyleAttribute("padding", KLC_HORIZONTAL_SPACING, SizeUnit::dip);
			content.SetStyleAttribute("margin", KLC_HORIZONTAL_SPACING, SizeUnit::dip);

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
		KxSciter::Element link = event.GetElement();
		SendEvent(IWebView::EvtNavigating, link.GetAttribute("href"), link.GetAttribute("target"));
	}
	void Sciter::OnSpoilerButton(KxSciter::BehaviorEvent& event)
	{
		if (KxSciter::Element content = event.GetElement().GetParent().SelectAny("div.bbc_spoiler_content"))
		{
			KxSciter::Element button = event.GetElement();
			if (content.GetStyleAttribute("display") == "block")
			{
				content.SetStyleAttribute("display", "none");
				button.SetText(KTr(KxID_OPEN));
			}
			else
			{
				content.SetStyleAttribute("display", "block");
				button.SetText(KTr(KxID_CLOSE));
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
		Host::SetWindowRenderer(KxSciter::WindowRenderer::DirectX);
		if (WindowWrapper::Create(parent, KxID_NONE, wxDefaultPosition, wxDefaultSize, style))
		{
			Bind(KxSciter::BehaviorEvent::EvtHyperlinkClick, &Sciter::OnHyperlink, this);
			Bind(KxSciter::BehaviorEvent::EvtDocumentReady, &Sciter::OnLoaded, this);

			m_SpoilerButtonHandler.Bind(KxSciter::BehaviorEvent::EvtButtonPress, &Sciter::OnSpoilerButton, this);
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
