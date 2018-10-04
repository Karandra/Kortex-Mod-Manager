#pragma once
#include "stdafx.h"
#include <wx/popupwin.h>
class KNotification;

class KNotificationPopup: public wxPopupWindow
{
	private:
		KNotification* m_Notification = NULL;

	private:
		void CreateUI();

	public:
		KNotificationPopup(KNotification* notification);

	public:
		virtual bool Destroy() override;
};
