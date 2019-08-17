#include "stdafx.h"
#include "BaseNotification.h"
#include "TrayPopup.h"
#include <Kortex/Application.hpp>
#include "Utility/Log.h"
#include <KxFramework/KxSystem.h>
#include <windowsx.h>

namespace
{
	Kortex::Notifications::TrayPopupHandler g_PopupHandler;

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
	void TrayPopupHandler::OnNotificationEvent(int eventID, int iconID, const wxPoint& pos)
	{
		switch (eventID)
		{
			case NIN_BALLOONSHOW:
			{
				m_IsDisplayed = true;
				break;
			}
			case NIN_BALLOONHIDE:
			case NIN_BALLOONTIMEOUT:
			{
				if (m_IsDisplayed && !m_SuppressDismiss)
				{
					Dismiss();
				}

				m_SuppressDismiss = false;
				m_IsDisplayed = false;
				break;
			}
			case NIN_BALLOONUSERCLICK:
			{
				break;
			}
		};
	}
	void TrayPopupHandler::InitNotifyInfo()
	{
		m_NotifyInfo.cbSize = sizeof(m_NotifyInfo);
		m_NotifyInfo.hWnd = m_Handle;
		m_NotifyInfo.uID = m_NotifyIconID;
		m_NotifyInfo.uCallbackMessage = m_NotifyMessage;
		m_NotifyInfo.guidItem = m_NotifyGUID;
		m_NotifyInfo.uVersion = NOTIFYICON_VERSION_4;
		m_NotifyInfo.uFlags = NIF_STATE|NIF_INFO|NIF_TIP|NIF_MESSAGE|NIF_SHOWTIP|NIF_GUID;
		m_NotifyInfo.dwInfoFlags = NIIF_LARGE_ICON;
		m_NotifyInfo.dwState = NIS_SHAREDICON;

		// Tray icon
		if (m_TrayIcon = ImageProvider::GetIcon(wxS("kortex-logo-icon")); m_TrayIcon.IsOk())
		{
			m_NotifyInfo.uFlags |= NIF_ICON;
			m_NotifyInfo.hIcon = m_TrayIcon.GetHICON();
		}
	}

	void TrayPopupHandler::InitMessages()
	{
		m_NotifyIconID = 1;
		m_NotifyMessage = WM_APP + 1;
	}
	void TrayPopupHandler::InitGUID()
	{
		AppOption option = Application::GetGlobalOptionOf<IApplication>().QueryOrCreateElement("NotificationGUID");
		if (m_NotifyGUID.FromString(option.GetValue()) != KxUUIDStatus::OK)
		{
			m_NotifyGUID.Create();
		}
		option.SetValue(m_NotifyGUID.ToString());
	}
	void TrayPopupHandler::CreateWindowClass()
	{
		m_WindowClass.cbSize = sizeof(m_WindowClass);
		m_WindowClass.lpszClassName = L"Kortex/TrayPopupMessageWindow";
		m_WindowClass.lpfnWndProc = [](HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
		{
			TrayPopupHandler* self = reinterpret_cast<TrayPopupHandler*>(::GetWindowLongPtrW(handle, GWLP_USERDATA));
			if (self && message == self->m_NotifyMessage)
			{
				self->OnNotificationEvent(LOWORD(lParam), HIWORD(lParam), wxPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
				return (LRESULT)0;
			}
			return ::DefWindowProcW(handle, message, wParam, lParam);
		};

		m_Atom = ::RegisterClassExW(&m_WindowClass);
		if (!m_Atom)
		{
			Utility::Log::LogError("Failed to register window class '%1': %2", m_WindowClass.lpszClassName, KxSystem::GetLastErrorMessage());
		}
	}
	void TrayPopupHandler::CreateWindow()
	{
		m_Handle = ::CreateWindowExW(0, m_WindowClass.lpszClassName, m_WindowClass.lpszClassName, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);
		::SetWindowLongPtrW(m_Handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

		if (!m_Handle)
		{
			Utility::Log::LogError("Failed to create window of class '%1': %2", m_WindowClass.lpszClassName, KxSystem::GetLastErrorMessage());
		}
	}

	void TrayPopupHandler::Popup(const INotification& notification)
	{
		const IApplication* app = IApplication::GetInstance();

		// Text
		CopyTextEllipsize(notification.GetCaption(), m_NotifyInfo.szInfoTitle);
		CopyTextEllipsize(notification.GetMessage(), m_NotifyInfo.szInfo);
		CopyTextEllipsize(KxString::Format(wxS("%1: %2"), app->GetShortName(), notification.GetCaption()), m_NotifyInfo.szTip);

		// Icon
		if (wxBitmap bitmap = notification.GetBitmap(); bitmap.IsOk())
		{
			m_NotifyIcon.CopyFromBitmap(bitmap);

			m_NotifyInfo.dwInfoFlags |= NIIF_USER;
			m_NotifyInfo.hBalloonIcon = m_NotifyIcon.GetHICON();
		}
		else
		{
			m_NotifyInfo.dwInfoFlags &= ~NIIF_USER;
			m_NotifyInfo.hBalloonIcon = nullptr;
		}

		if (m_IsDisplayed)
		{
			m_SuppressDismiss = true;
			::Shell_NotifyIconW(NIM_MODIFY, &m_NotifyInfo);
		}
		else
		{
			m_SuppressDismiss = false;
			::Shell_NotifyIconW(NIM_SETVERSION, &m_NotifyInfo);
			::Shell_NotifyIconW(NIM_ADD, &m_NotifyInfo);
		}
	}
	void TrayPopupHandler::Dismiss()
	{
		::Shell_NotifyIconW(NIM_DELETE, &m_NotifyInfo);
	}

	void TrayPopupHandler::Create()
	{
		if (!m_Handle)
		{
			InitGUID();
			InitMessages();

			CreateWindowClass();
			CreateWindow();

			InitNotifyInfo();
		}
	}
	void TrayPopupHandler::Destroy()
	{
		if (m_Handle)
		{
			::DestroyWindow(m_Handle);
			::UnregisterClassW(m_WindowClass.lpszClassName, m_WindowClass.hInstance);

			m_Handle = nullptr;
			m_Atom = 0;
			m_IsDisplayed = false;
		}
	}
}

namespace Kortex::Notifications
{
	void BaseNotification::DoDestroy()
	{
		if (auto popup = std::move(m_Popup))
		{
			popup->Destroy();
		}
	}

	void BaseNotification::Popup()
	{
		if (!g_PopupHandler.GetHandle())
		{
			g_PopupHandler.Create();
		}

		m_Popup = std::make_unique<TrayPopup>(*this, g_PopupHandler);
		m_Popup->Popup();
	}
	bool BaseNotification::HasPopup() const
	{
		return m_Popup != nullptr;
	}
	void BaseNotification::DestroyPopup()
	{
		DoDestroy();
	}
}
