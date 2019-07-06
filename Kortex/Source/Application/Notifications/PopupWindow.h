#pragma once
#include "stdafx.h"
#include <wx/popupwin.h>

namespace Kortex
{
	class INotification;
}

namespace Kortex::Notifications
{
	class PopupWindow: public wxPopupWindow
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
			void Popup();
			void Dismiss();
	};
}
