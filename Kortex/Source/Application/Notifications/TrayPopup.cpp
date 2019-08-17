#include "stdafx.h"
#include "TrayPopup.h"
#include <Kortex/Application.hpp>
#include <Kortex/Notification.hpp>
#include "UI/KMainWindow.h"
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxStaticBitmap.h>
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxSystem.h>

namespace
{
	constexpr auto g_NotifyID = 1;
	constexpr auto g_NotifyMessage = WM_APP + 1;
	
	template<class T> void CopyTextEllipsize(wxString&& value, T& buffer)
	{
		if (value.length() >= std::size(buffer))
		{
			value.Truncate(std::size(buffer) - 4);
			value += wxS("...");
		}
		wcsncpy_s(buffer, value.wc_str(), value.length());
	}
}

namespace Kortex::Notifications
{
	LRESULT CALLBACK TrayPopupHandler::WindowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == g_NotifyMessage)
		{
			TrayPopupHandler& self = *reinterpret_cast<TrayPopupHandler*>(::GetWindowLongPtrW(handle, GWLP_USERDATA));
			const uint16_t eventID = LOWORD(lParam);
			const uint16_t iconID = HIWORD(lParam);

			switch (eventID)
			{
				case NIN_BALLOONUSERCLICK:
				{
					self.m_Popup.OnBallonClick();
					break;
				}
				case NIN_BALLOONTIMEOUT:
				{
					self.m_Popup.OnBallonTimeout();
					break;
				}
				case NIN_BALLOONSHOW:
				{
					self.m_Popup.OnBallonShow();
					break;
				}
				case NIN_BALLOONHIDE:
				{
					self.m_Popup.OnBallonHide();
					break;
				}
			};
			return 0;
		}
		return DefWindowProcW(handle, message, wParam, lParam);
	}
	
	TrayPopupHandler::TrayPopupHandler(TrayPopup& popup)
		:m_Popup(popup)
	{
		m_WindowClass.cbSize = sizeof(m_WindowClass);
		m_WindowClass.lpszClassName = L"Kortex/TrayPopupMessageWindow";
		m_WindowClass.lpfnWndProc = WindowProc;
		m_Atom = ::RegisterClassExW(&m_WindowClass);

		m_Handle = ::CreateWindowExW(0, m_WindowClass.lpszClassName, L"", 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);
		::SetWindowLongPtrW(m_Handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	}
	TrayPopupHandler::~TrayPopupHandler()
	{
		Destroy();
	}

	void TrayPopupHandler::Destroy()
	{
		if (m_Handle)
		{
			::DestroyWindow(m_Handle);
			::UnregisterClassW(m_WindowClass.lpszClassName, m_WindowClass.hInstance);

			m_Handle = nullptr;
			m_Atom = 0;
		}
	}
}

namespace Kortex::Notifications
{
	void TrayPopup::CreateUI()
	{
		m_Info.cbSize = sizeof(m_Info);
		m_Info.hWnd = m_Handler.GetHandle();
		m_Info.uID = g_NotifyID;
		m_Info.guidItem = m_GUID.GetID();
		m_Info.uCallbackMessage = g_NotifyMessage;
		m_Info.uVersion = NOTIFYICON_VERSION_4;
		m_Info.uFlags = NIF_STATE|NIF_INFO|NIF_TIP|NIF_MESSAGE|NIF_SHOWTIP|NIF_GUID;
		m_Info.dwInfoFlags = NIIF_LARGE_ICON;
		m_Info.dwState = NIS_SHAREDICON;

		// Text
		CopyTextEllipsize(m_Notification->GetCaption(), m_Info.szInfoTitle);
		CopyTextEllipsize(m_Notification->GetMessage(), m_Info.szInfo);
		CopyTextEllipsize(IApplication::GetInstance()->GetName(), m_Info.szTip);

		// Balloon icon
		if (wxBitmap bitmap = m_Notification->GetBitmap(); bitmap.IsOk())
		{
			m_Icon.CopyFromBitmap(bitmap);

			m_Info.dwInfoFlags |= NIIF_USER;
			m_Info.hBalloonIcon = m_Icon.GetHICON();
		}

		// Tray icon
		if (m_TrayIcon = ImageProvider::GetIcon(wxS("kortex-logo-icon")); m_TrayIcon.IsOk())
		{
			m_Info.uFlags |= NIF_ICON;
			m_Info.hIcon = m_TrayIcon.GetHICON();
		}
	}
	void TrayPopup::DoDestroy()
	{
		if (m_Notification)
		{
			m_Notification = nullptr;
			DoDismiss();
		}
		if (m_Handler.GetHandle())
		{
			m_Handler.Destroy();
		}
	}
	void TrayPopup::DoDismiss()
	{
		::Shell_NotifyIconW(NIM_DELETE, &m_Info);
	}

	void TrayPopup::OnBallonClick()
	{
	}
	void TrayPopup::OnBallonTimeout()
	{
		Dismiss();
	}
	void TrayPopup::OnBallonShow()
	{
	}
	void TrayPopup::OnBallonHide()
	{
		Dismiss();
	}

	TrayPopup::TrayPopup(INotification& notification, const KxUUID& guid)
		:m_Notification(&notification), m_GUID(guid), m_Handler(*this)
	{
		CreateUI();
	}
	TrayPopup::~TrayPopup()
	{
		DoDestroy();
	}

	void TrayPopup::Popup()
	{
		if (m_Notification)
		{
			if (!::Shell_NotifyIconW(NIM_MODIFY, &m_Info))
			{
				::Shell_NotifyIconW(NIM_SETVERSION, &m_Info);
				::Shell_NotifyIconW(NIM_ADD, &m_Info);
			}
		}
	}
	void TrayPopup::Dismiss()
	{
		if (m_Notification)
		{
			DoDismiss();
		}
	}
	void TrayPopup::Destroy()
	{
		DoDestroy();
	}
}
