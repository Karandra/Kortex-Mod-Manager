#include "stdafx.h"
#include "BaseNotification.h"
#include "PopupWindow.h"
#include "TrayPopup.h"
#include <Kortex/Application.hpp>
#include <Kx/Async/Coroutine.h>

namespace
{
	KxUUID g_NotificationGUID;
}

namespace Kortex::Notifications
{
	void BaseNotification::UsePopupWindow()
	{
		m_Coroutine = KxCoroutine::Run([this](KxCoroutine& coroutine)
		{
			if (!m_Popup)
			{
				const HWND previousForegroundWindow = ::GetForegroundWindow();

				m_Popup = std::make_unique<PopupWindow>(*this);
				m_Popup->Popup();

				if (previousForegroundWindow)
				{
					::SetForegroundWindow(previousForegroundWindow);
				}
				return coroutine.YieldWait(wxTimeSpan::Milliseconds(3000));
			}
			else
			{
				if (static_cast<PopupWindow&>(*m_Popup).GetWindow()->IsMouseInWindow())
				{
					return KxCoroutine::YieldWait(wxTimeSpan::Milliseconds(1500));
				}
				else
				{
					DestroyPopup();
					return coroutine.YieldStop();
				}
			}
		});
	}
	void BaseNotification::UseTray()
	{
		if (g_NotificationGUID.IsNull())
		{
			IAppOption option = Application::GetGlobalOptionOf<IApplication>().QueryOrCreateElement("NotificationGUID");

			if (g_NotificationGUID.FromString(option.GetValue()) != KxUUIDStatus::OK)
			{
				g_NotificationGUID.Create();
			}
			option.SetValue(g_NotificationGUID.ToString());
		}

		m_Popup = std::make_unique<TrayPopup>(*this, g_NotificationGUID);
		m_Popup->Popup();
	}

	void BaseNotification::DoDestroy()
	{
		if (KxCoroutine* coroutine = m_Coroutine)
		{
			m_Coroutine = nullptr;
			coroutine->Terminate();
		}
		if (auto popup = std::move(m_Popup))
		{
			popup->Destroy();
		}
	}

	void BaseNotification::Popup()
	{
		switch (m_Type)
		{
			case PopupType::PopupWindow:
			{
				UsePopupWindow();
				break;
			}
			case PopupType::Tray:
			{
				UseTray();
				break;
			}
		};
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
