#include "stdafx.h"
#include "BaseNotification.h"
#include <Kortex/Notification.hpp>
#include <KxFramework/KxCoroutine.h>

namespace Kortex::Notifications
{
	BaseNotification::~BaseNotification()
	{
		if (HasPopupWindow())
		{
			DestroyPopupWindow();
		}
	}

	void BaseNotification::SetPopupWindow(Notifications::PopupWindow* window)
	{
		m_PopupWindow = window;
	}

	void BaseNotification::ShowPopupWindow()
	{
		m_PopupWindowCoroutine = KxCoroutine::Run([this](KxCoroutine& coroutine)
		{
			if (m_PopupWindow)
			{
				if (m_PopupWindow->IsMouseInWindow())
				{
					return KxCoroutine::YieldWait(wxTimeSpan::Milliseconds(1500));
				}
				else
				{
					DestroyPopupWindow();

					m_PopupWindowCoroutine = nullptr;
					return coroutine.YieldStop();
				}
			}
			else
			{
				const HWND previousForegroundWindow = ::GetForegroundWindow();

				m_PopupWindow = new Notifications::PopupWindow(*this);
				m_PopupWindow->Popup();

				if (previousForegroundWindow)
				{
					::SetForegroundWindow(previousForegroundWindow);
				}
				return coroutine.YieldWait(wxTimeSpan::Milliseconds(3000));
			}
		});
	}
	bool BaseNotification::HasPopupWindow() const
	{
		return m_PopupWindow && !wxTheApp->IsScheduledForDestruction(m_PopupWindow) && !m_PopupWindow->IsBeingDeleted();
	}
	void BaseNotification::DestroyPopupWindow()
	{
		if (m_PopupWindowCoroutine)
		{
			m_PopupWindowCoroutine->Terminate();
			m_PopupWindowCoroutine = nullptr;
		}
		if (m_PopupWindow)
		{
			m_PopupWindow->Destroy();
			m_PopupWindow = nullptr;
		}
	}
}
