#pragma once
#include "stdafx.h"
#include "Application/INotification.h"
#include <wx/popupwin.h>

namespace Kortex::Notifications
{
	class PopupWindow: public wxPopupWindow, public INotificationPopup
	{
		private:
			const INotification* m_Notification = nullptr;
			wxSize m_Size;
			wxSize m_Margin;

		private:
			void CreateUI();

		public:
			PopupWindow(const INotification& notification);

		public:
			void Popup() override;
			void Dismiss() override;
	};
}
