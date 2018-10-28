#include "stdafx.h"
#include "KNotificationCenter.h"
#include "KNotificationPopup.h"
#include "KNotificationPopupList.h"
#include <KxFramework/KxCoroutine.h>
#include <KxFramework/KxAuiToolBar.h>
#include <wx/popupwin.h>

void KNotificationCenter::OnSetToolBarButton(KxAuiToolBarItem* button)
{
	m_Button = button;
	UpdateToolBarButton();

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	m_PopupListWindow = new wxPopupTransientWindow(m_Button->GetToolBar(), wxBORDER_NONE);
	m_PopupListWindow->SetSizer(sizer);
	m_PopupListWindow->SetInitialSize(wxSize(325, 425));

	m_PopupListModel = new KNotificationPopupList();
	m_PopupListModel->Create(m_PopupListWindow, sizer);
}
void KNotificationCenter::OnToolBarButton(KxAuiToolBarEvent& event)
{
	wxPoint pos = m_Button->GetToolBar()->ClientToScreen(m_Button->GetDropdownMenuPosition());
	const int offset = 10;
	const int screenWidth = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
	const int rightSide = pos.x + m_PopupListWindow->GetSize().GetWidth();
	if (rightSide > screenWidth)
	{
		pos.x -= (rightSide - screenWidth) + offset;
	}

	m_PopupListWindow->Layout();
	m_PopupListWindow->SetPosition(pos);

	m_PopupListModel->RefreshItems();
	m_PopupListModel->OnShowWindow(m_PopupListWindow);

	m_PopupListWindow->Popup(wxWindow::FindFocus());
}
void KNotificationCenter::UpdateToolBarButton()
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

void KNotificationCenter::DoNotify(KNotification* notification)
{
	KEvent::CallAfter([this, notification]()
	{
		m_Notifications.emplace(m_Notifications.begin(), notification);
		
		UpdateToolBarButton();
		if (m_PopupListWindow->IsShown())
		{
			m_PopupListModel->RefreshItems();
		}
		notification->ShowPopupWindow();
	});
}

KNotificationCenter::KNotificationCenter()
{
}
KNotificationCenter::~KNotificationCenter()
{
}

wxString KNotificationCenter::GetID() const
{
	return "KNotificationCenter";
}
wxString KNotificationCenter::GetName() const
{
	return "KNotificationCenter";
}
wxString KNotificationCenter::GetVersion() const
{
	return "1.0";
}
KImageEnum KNotificationCenter::GetImageID() const
{
	return KIMG_BELL;
}

bool KNotificationCenter::HasActivePopups() const
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
size_t KNotificationCenter::GetActivePopupsCount() const
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
