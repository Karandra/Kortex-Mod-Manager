#include "stdafx.h"
#include "InternetExplorer.h"
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxTaskDialog.h>
#include <wx/clipbrd.h>
#include <mshtml.h>
#include <mshtmcid.h>
#include <comutil.h>
#include <ExDisp.h>

namespace
{
	void CopyTextToClipboard(const wxString& value)
	{
		if (wxTheClipboard->Open())
		{
			wxTheClipboard->SetData(new wxTextDataObject(value));
			wxTheClipboard->Close();
		}
	}
	template<class T> KxCOMPtr<T> GetIHTMLDocument(IWebBrowser2& webBrowser)
	{
		KxCOMPtr<IDispatch> dispatch;
		KxCOMPtr<T> document;

		const HRESULT result = webBrowser.get_Document(&dispatch);
		if (dispatch && SUCCEEDED(result))
		{
			dispatch->QueryInterface(&document);
		}
		return document;
	}
}

namespace Kortex::UI::WebViewBackend
{
	void InternetExplorer::DoLoadPage(const wxString& html)
	{
		m_WebView->Unbind(wxEVT_WEBVIEW_NAVIGATING, &InternetExplorer::OnNavigating, this);
		m_WebView->SetPage(html, wxWebViewDefaultURLStr);
	}
	void InternetExplorer::DoLoadURL(const wxString& url)
	{
		m_WebView->Unbind(wxEVT_WEBVIEW_NAVIGATING, &InternetExplorer::OnNavigating, this);
		m_WebView->LoadURL(url);
	}

	void InternetExplorer::OnNavigating(wxWebViewEvent& event)
	{
		const wxString& url = event.GetURL();
		if (!url.IsEmpty() && url != wxWebViewDefaultURLStr)
		{
			event.SetEventType(IWebView::EvtNavigating);
			m_EvtHandler.ProcessEvent(event);
		}
	}
	void InternetExplorer::OnNavigated(wxWebViewEvent& event)
	{
		m_WebView->Bind(wxEVT_WEBVIEW_NAVIGATING, &InternetExplorer::OnNavigating, this);
		m_IsEmpty = m_WebView->GetPageText().IsEmpty();

		event.SetEventType(IWebView::EvtNavigated);
		m_EvtHandler.ProcessEvent(event);
	}
	void InternetExplorer::OnLoaded(wxWebViewEvent& event)
	{
		event.SetEventType(IWebView::EvtLoaded);
		m_EvtHandler.ProcessEvent(event);
	}
	void InternetExplorer::OnError(wxWebViewEvent& event)
	{
		event.SetEventType(IWebView::EvtError);
		m_EvtHandler.ProcessEvent(event);
	}

	IWebBrowser2& InternetExplorer::GetWebBrowser() const
	{
		return *reinterpret_cast<IWebBrowser2*>(m_WebView->GetNativeBackend());
	}
	KxCOMPtr<IHTMLDocument2> InternetExplorer::GetDocument2() const
	{
		return GetIHTMLDocument<IHTMLDocument2>(GetWebBrowser());
	}
	bool InternetExplorer::ExecCommand(const wxString& command, const wxAny& arg)
	{
		if (auto document = GetDocument2())
		{
			_bstr_t commandBstr(command.wc_str());
			VARIANT_BOOL showUI = VARIANT_FALSE;
			VARIANT_BOOL ret = VARIANT_FALSE;

			if (arg.IsNull())
			{
				document->execCommand(commandBstr, showUI, {}, &ret);
			}
			else if (bool value; arg.GetAs(&value))
			{
				document->execCommand(commandBstr, showUI, _variant_t(value), &ret);
			}
			else if (int32_t value; arg.GetAs(&value))
			{
				document->execCommand(commandBstr, showUI, _variant_t(value), &ret);
			}
			else if (int64_t value; arg.GetAs(&value))
			{
				document->execCommand(commandBstr, showUI, _variant_t(value), &ret);
			}
			else if (float value; arg.GetAs(&value))
			{
				document->execCommand(commandBstr, showUI, _variant_t(value), &ret);
			}
			else if (double value; arg.GetAs(&value))
			{
				document->execCommand(commandBstr, showUI, _variant_t(value), &ret);
			}
			else if (wxString value; arg.GetAs(&value))
			{
				document->execCommand(commandBstr, showUI, _variant_t(value.wc_str()), &ret);
			}
			return ret == VARIANT_TRUE;
		}
		return false;
	}

	InternetExplorer::InternetExplorer(wxWindow* parent, wxEvtHandler& evthandler, long style)
		:m_EvtHandler(evthandler)
	{
		m_WebView = wxWebView::New(parent, KxID_NONE, wxWebViewDefaultURLStr, wxDefaultPosition, wxDefaultSize, wxWebViewBackendDefault, style);
		m_WebView->EnableContextMenu(false);
		m_WebView->EnableHistory(false);
		m_WebView->SetEditable(false);

		m_WebView->Bind(wxEVT_WEBVIEW_NAVIGATED, &InternetExplorer::OnNavigated, this);
		m_WebView->Bind(wxEVT_WEBVIEW_LOADED, &InternetExplorer::OnLoaded, this);
		m_WebView->Bind(wxEVT_WEBVIEW_ERROR, &InternetExplorer::OnError, this);
	}

	bool InternetExplorer::LoadText(const wxString& text)
	{
		const KxColor fgColor = m_WebView->GetForegroundColour();
		const KxColor bgColor = m_WebView->GetBackgroundColour();
		const wxFont font = m_WebView->GetFont();

		auto FormatElement = [&](const wxString& html)
		{
			const wxString css = KxString::Format(wxS("font-family: '%1', sans-serif; font-size: %2pt; color: %3; background: %4; background-color: %4;"),
												  font.GetFaceName(),
												  font.GetPointSize(),
												  fgColor.ToString(KxColor::C2S::CSS, KxColor::C2SAlpha::Auto),
												  bgColor.ToString(KxColor::C2S::CSS, KxColor::C2SAlpha::Auto)
			);
			return KxString::Format(wxS("<html><body style=\"%1\">%2</body></html>"), css, html);
		};

		DoLoadPage(FormatElement(KxHTMLWindow::ProcessPlainText(text)));
		return true;
	}
}
