#include "stdafx.h"
#include "DefaultNotificationCenter.h"
#include "Application/Resources/ImageResourceID.h"
#include <Kortex/Application.hpp>
#include <Kortex/Notification.hpp>
#include <Kortex/Events.hpp>
#include "Utility/Log.h"
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
			m_Button->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::BellRedCircle));
		}
		else
		{
			m_Button->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Bell));
		}
	}

	void DefaultNotificationCenter::DoNotify(INotification* notification)
	{
		Utility::Log::LogInfo("DefaultNotificationCenter::DoNotify");
		Utility::Log::LogInfo("Caption: %1", notification->GetCaption());
		Utility::Log::LogInfo("Message: %1", notification->GetMessage());

		INotification& notificationRef = *m_Notifications.emplace(m_Notifications.begin(), notification)->get();
		IEvent::CallAfter([this, &notificationRef]()
		{
			UpdateToolBarButton();
			if (m_PopupWindow->IsShown())
			{
				m_PopupDisplayModel->RefreshItems();
			}
			notificationRef.ShowPopupWindow();
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
