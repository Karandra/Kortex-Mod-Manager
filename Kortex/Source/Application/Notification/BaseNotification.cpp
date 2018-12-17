#include "stdafx.h"
#include "BaseNotification.h"
#include <Kortex/Notification.hpp>
#include <KxFramework/KxCoroutine.h>

namespace Kortex::Notification
{
	BaseNotification::~BaseNotification()
	{
		if (HasPopupWindow())
		{
			DestroyPopupWindow();
		}
	}

	void BaseNotification::DestroyPopupWindow()
	{
		wxApp::GetInstance()->ScheduleForDestruction(m_PopupWindow);
	}
	void BaseNotification::SetPopupWindow(Notification::PopupWindow* window)
	{
		m_PopupWindow = window;
	}

	void BaseNotification::ShowPopupWindow()
	{
		KxCoroutine::Run([this](KxCoroutineBase& coroutine)
		{
			if (m_PopupWindow)
			{
				if (m_PopupWindow->IsMouseInWindow())
				{
					return coroutine.YieldWaitSeconds(1.5);
				}
				else
				{
					DestroyPopupWindow();
					m_PopupWindow = nullptr;
					return coroutine.YieldStop();
				}
			}
			else
			{
				HWND previousForegroundWindow = ::GetForegroundWindow();

				m_PopupWindow = new Notification::PopupWindow(this);
				m_PopupWindow->Show();

				if (previousForegroundWindow)
				{
					::SetForegroundWindow(previousForegroundWindow);
				}
				return coroutine.YieldWaitSeconds(3);
			}
		});
	}
	bool BaseNotification::HasPopupWindow() const
	{
		return m_PopupWindow && !wxTheApp->IsScheduledForDestruction(m_PopupWindow) && !m_PopupWindow->IsBeingDeleted();
	}
}
