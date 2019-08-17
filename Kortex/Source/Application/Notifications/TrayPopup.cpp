#include "stdafx.h"
#include "TrayPopup.h"
#include "BaseNotification.h"

namespace Kortex::Notifications
{
	void TrayPopup::DoDestroy()
	{
		if (m_Notification)
		{
			m_Notification = nullptr;
			m_PopupHandler.Dismiss();
		}
	}

	TrayPopup::TrayPopup(INotification& notification, TrayPopupHandler& popupHandler)
		:m_Notification(&notification), m_PopupHandler(popupHandler)
	{
	}
	TrayPopup::~TrayPopup()
	{
		DoDestroy();
	}

	void TrayPopup::Popup()
	{
		if (m_Notification)
		{
			m_PopupHandler.Popup(*m_Notification);
		}
	}
	void TrayPopup::Dismiss()
	{
		if (m_Notification)
		{
			m_PopupHandler.Dismiss();
		}
	}
	void TrayPopup::Destroy()
	{
		DoDestroy();
	}
}
