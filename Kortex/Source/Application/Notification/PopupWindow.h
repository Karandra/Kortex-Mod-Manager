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
			INotification* m_Notification = nullptr;

		private:
			void CreateUI();

		public:
			PopupWindow(INotification* notification);

		public:
			virtual bool Destroy() override;
	};
}
