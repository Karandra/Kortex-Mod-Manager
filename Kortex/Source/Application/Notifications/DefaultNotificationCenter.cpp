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

namespace Kortex::Notifications
{
	void DefaultNotificationCenter::UpdateLabel()
	{
		if (m_Notifications.empty())
		{
			m_PpoupToolbar_Label->SetLabel(KTr("NotificationCenter.NameShort"));
		}
		else
		{
			m_PpoupToolbar_Label->SetLabel(KxString::Format(wxS("%1 (%2)"), KTr("NotificationCenter.NameShort"), m_Notifications.size()));
		}
		m_PpoupToolbar->UpdateUI();
	}
	void DefaultNotificationCenter::OnClearNotifications(KxAuiToolBarEvent& event)
	{
		ClearNotifications();
	}
	void DefaultNotificationCenter::OnNotificationsCountChanged()
	{
		if (IsNotificationsDisplayed())
		{
			m_PopupDisplayModel->RefreshItems();
			UpdateLabel();
		}
	}

	void DefaultNotificationCenter::OnSetToolBarButton(KxAuiToolBarItem* button)
	{
		m_Button = button;
		m_Button->SetShortHelp(KTr("NotificationCenter.Name"));
		UpdateToolBarButton();

		// Popup window
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		m_PopupWindow = new wxPopupTransientWindow(m_Button->GetToolBar(), wxBORDER_THEME);
		IThemeManager::GetActive().ProcessWindow(m_PopupWindow);
		m_PopupWindow->SetSizer(sizer);
		m_PopupWindow->SetInitialSize(wxSize(325, 425));

		// Toolbar
		m_PpoupToolbar = new KxAuiToolBar(m_PopupWindow, KxID_NONE);
		m_PpoupToolbar_Label = m_PpoupToolbar->AddLabel(wxEmptyString);
		m_PpoupToolbar->AddStretchSpacer();
		
		{
			m_PpoupToolbar_ClearNotifications = m_PpoupToolbar->AddTool(wxEmptyString, ImageProvider::GetBitmap(ImageResourceID::Broom));
			m_PpoupToolbar_ClearNotifications->SetEnabled(false);
			m_PpoupToolbar_ClearNotifications->SetShortHelp(KTr("NotificationCenter.ClearNotifications"));
			m_PpoupToolbar_ClearNotifications->Bind(KxEVT_AUI_TOOLBAR_CLICK, &DefaultNotificationCenter::OnClearNotifications, this);
		}

		m_PpoupToolbar->Realize();
		m_PpoupToolbar->SetBackgroundColour(m_PopupWindow->GetBackgroundColour());
		sizer->Add(m_PpoupToolbar, 0, wxEXPAND);

		// Notifications list
		m_PopupDisplayModel = new DisplayModel();
		m_PopupDisplayModel->CreateView(m_PopupWindow);
		sizer->Add(m_PopupDisplayModel->GetView(), 1, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING_SMALL);
	}
	void DefaultNotificationCenter::OnToolBarButton(KxAuiToolBarEvent& event)
	{
		if (IsNotificationsDisplayed())
		{
			HideNotificationsWindow();
		}
		else
		{
			ShowNotificationsWindow();
		}
	}
	void DefaultNotificationCenter::UpdateToolBarButton()
	{
		const bool isEmpty = m_Notifications.empty();

		if (m_Button)
		{
			if (!isEmpty)
			{
				m_Button->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::BellRedCircle));
			}
			else
			{
				m_Button->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Bell));
			}
		}
		if (m_PpoupToolbar_ClearNotifications)
		{
			m_PpoupToolbar_ClearNotifications->SetEnabled(!isEmpty);
		}
	}

	void DefaultNotificationCenter::QueueNotification(std::unique_ptr<INotification> notification)
	{
		Utility::Log::LogInfo("DefaultNotificationCenter::DoNotify");
		Utility::Log::LogInfo("Caption: %1", notification->GetCaption());
		Utility::Log::LogInfo("Message: %1", notification->GetMessage());

		INotification& ref = *notification;
		if (wxCriticalSectionLocker lock(m_NotificationsCS); true)
		{
			m_Notifications.emplace(m_Notifications.begin(), std::move(notification));
		}

		IEvent::CallAfter([this, &ref]()
		{
			UpdateToolBarButton();
			OnNotificationAdded(ref);
			ref.ShowPopupWindow();
		});
	}

	bool DefaultNotificationCenter::IsNotificationsDisplayed() const
	{
		return m_PopupWindow && m_PopupWindow->IsShown();
	}
	void DefaultNotificationCenter::ShowNotificationsWindow()
	{
		if (m_PopupWindow)
		{
			wxPoint pos = m_Button->GetToolBar()->ClientToScreen(m_Button->GetDropdownMenuPosition());
			const int offset = wxSystemSettings::GetMetric(wxSYS_SMALLICON_X);
			const int screenWidth = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
			const int rightSide = pos.x + m_PopupWindow->GetSize().GetWidth();
			if (rightSide > screenWidth)
			{
				pos.x -= (rightSide - screenWidth) + offset;
			}

			m_PopupWindow->Layout();
			m_PopupWindow->SetPosition(pos);
			m_PopupWindow->Popup(wxWindow::FindFocus());

			m_PopupDisplayModel->OnShowWindow();
			OnNotificationsCountChanged();
		}
	}
	void DefaultNotificationCenter::HideNotificationsWindow()
	{
		if (m_PopupWindow)
		{
			m_PopupWindow->Dismiss();
		}
	}
}
