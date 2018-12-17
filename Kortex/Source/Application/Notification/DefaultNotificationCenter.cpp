#include "stdafx.h"
#include "DefaultNotificationCenter.h"
#include <Kortex/Application.hpp>
#include <Kortex/Notification.hpp>
#include <Kortex/Events.hpp>
#include "KImageProvider.h"
#include <KxFramework/KxCoroutine.h>
#include <KxFramework/KxAuiToolBar.h>
#include <wx/popupwin.h>

namespace Kortex::Notification
{
	void DefaultNotificationCenter::OnSetToolBarButton(KxAuiToolBarItem* button)
	{
		m_Button = button;
		UpdateToolBarButton();

		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		m_PopupWindow = new wxPopupTransientWindow(m_Button->GetToolBar(), wxBORDER_NONE);
		m_PopupWindow->SetSizer(sizer);
		m_PopupWindow->SetInitialSize(wxSize(325, 425));

		m_PopupDisplayModel = new DisplayModel();
		m_PopupDisplayModel->Create(m_PopupWindow, sizer);
	}
	void DefaultNotificationCenter::OnToolBarButton(KxAuiToolBarEvent& event)
	{
		wxPoint pos = m_Button->GetToolBar()->ClientToScreen(m_Button->GetDropdownMenuPosition());
		const int offset = 10;
		const int screenWidth = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
		const int rightSide = pos.x + m_PopupWindow->GetSize().GetWidth();
		if (rightSide > screenWidth)
		{
			pos.x -= (rightSide - screenWidth) + offset;
		}

		m_PopupWindow->Layout();
		m_PopupWindow->SetPosition(pos);

		m_PopupDisplayModel->RefreshItems();
		m_PopupDisplayModel->OnShowWindow(m_PopupWindow);

		m_PopupWindow->Popup(wxWindow::FindFocus());
	}
	void DefaultNotificationCenter::UpdateToolBarButton()
	{
		if (!m_Notifications.empty())
		{
			m_Button->SetBitmap(KGetBitmap(KIMG_BELL_RED_CIRCLE));
		}
		else
		{
			m_Button->SetBitmap(KGetBitmap(KIMG_BELL));
		}
	}

	void DefaultNotificationCenter::DoNotify(INotification* notification)
	{
		IEvent::CallAfter([this, notification]()
		{
			m_Notifications.emplace(m_Notifications.begin(), notification);

			UpdateToolBarButton();
			if (m_PopupWindow->IsShown())
			{
				m_PopupDisplayModel->RefreshItems();
			}
			notification->ShowPopupWindow();
		});
	}

	bool DefaultNotificationCenter::HasActivePopups() const
	{
		for (const auto& notification: m_Notifications)
		{
			if (notification->HasPopupWindow())
			{
				return true;
			}
		}
		return false;
	}
	size_t DefaultNotificationCenter::GetActivePopupsCount() const
	{
		size_t count = 0;
		for (const auto& notification: m_Notifications)
		{
			if (notification->HasPopupWindow())
			{
				count++;
			}
		}
		return count;
	}
}
