#include "stdafx.h"
#include "WebView.h"
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxColor.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxMenu.h>
#include <KxFramework/KxCOM.h>
#include <wx/clipbrd.h>
#include <mshtml.h>
#include <mshtmcid.h>
#include <comutil.h>

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

namespace Kortex::UI
{
	void WebView::DoLoadPage(const wxString& html)
	{
		m_View->Unbind(wxEVT_WEBVIEW_NAVIGATING, &WebView::OnNavigate, this);
		m_View->SetPage(html, wxWebViewDefaultURLStr);
	}
	void WebView::DoLoadURL(const wxString& url)
	{
		m_View->Unbind(wxEVT_WEBVIEW_NAVIGATING, &WebView::OnNavigate, this);
		m_View->LoadURL(url);
	}

	void WebView::AskOpenURL(const wxString& url)
	{
		KxTaskDialog dialog(m_View, KxID_NONE, KTr("Generic.OpenWebSiteDialog.Caption"), url, KxBTN_YES|KxBTN_NO);
		dialog.SetOptionEnabled(KxTD_SIZE_TO_CONTENT);
		
		dialog.AddButton(KxID_COPY_LINK, KTr(KxID_COPY_LINK));
		dialog.Bind(KxEVT_STDDIALOG_BUTTON, [&url](wxNotifyEvent& event)
		{
			if (event.GetId() == KxID_COPY_LINK)
			{
				CopyTextToClipboard(url);
				event.Veto();
			}
			else
			{
				event.Skip();
			}
		});

		if (dialog.ShowModal() == KxID_YES)
		{
			KxShell::Execute(m_View, url, "open");
		}
	}
	void WebView::OnNavigate(wxWebViewEvent& event)
	{
		AskOpenURL(event.GetURL());
		event.Veto();
	}
	void WebView::OnNavigated(wxWebViewEvent& event)
	{
		m_View->Bind(wxEVT_WEBVIEW_NAVIGATING, &WebView::OnNavigate, this);
		m_IsEmpty = m_View->GetPageText().IsEmpty();

		event.Skip();
	}

	IWebBrowser2& WebView::GetWebBrowser() const
	{
		return *reinterpret_cast<IWebBrowser2*>(m_View->GetNativeBackend());
	}
	KxCOMPtr<IHTMLDocument2> WebView::GetDocument2() const
	{
		return GetIHTMLDocument<IHTMLDocument2>(GetWebBrowser());
	}
	bool WebView::ExecCommand(const wxString& command, const wxAny& arg)
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

	bool WebView::Create(wxWindow* parent, long style)
	{
		if (parent)
		{
			m_View = wxWebView::New(parent, KxID_NONE, wxWebViewDefaultURLStr, wxDefaultPosition, wxDefaultSize, wxWebViewBackendDefault, style);
			m_View->EnableContextMenu(false);
			m_View->EnableHistory(false);
			m_View->SetEditable(false);

			m_View->Bind(wxEVT_WEBVIEW_NAVIGATED, &WebView::OnNavigated, this);
			return true;
		}
		return false;
	}

	void WebView::LoadText(const wxString& text)
	{
		const KxColor fgColor = m_View->GetForegroundColour();
		const KxColor bgColor = m_View->GetBackgroundColour();
		const wxFont font = m_View->GetFont();

		const wxString css = KxString::Format(wxS("font-family: '%1', sans-serif; font-size: %2pt; color: %3; background-color: %4;"),
											  font.GetFaceName(),
											  font.GetPointSize(),
											  fgColor.ToString(KxColor::C2S::CSS, KxColor::C2SAlpha::Auto),
											  bgColor.ToString(KxColor::C2S::CSS, KxColor::C2SAlpha::Auto)
		);
		const wxString html = KxString::Format(wxS("<html><body style=\"%1\">%2</body></html>"),
											   css,
											   KxHTMLWindow::ProcessPlainText(text)
		);

		DoLoadPage(html);
	}
	void WebView::LoadURL(const wxString& url)
	{
		DoLoadURL(url);
	}
}
