#pragma once
#include "stdafx.h"
#include "Application/INotification.h"
class wxPopupWindow;

namespace Kortex::Notifications
{
	class PopupWindow: public wxObject, public INotificationPopup
	{
		private:
			INotification* m_Notification = nullptr;

			wxNativeWindow m_DesktopWindow;
			wxPopupWindow* m_Window = nullptr;
			wxSize m_Size;
			wxSize m_Margin;

		private:
			void CreateUI();
			void DoDestroy();

		public:
			PopupWindow(INotification& notification);
			~PopupWindow();

		public:
			void Popup() override;
			void Dismiss() override;
			void Destroy() override;

			wxWindow* GetWindow() const;
	};
}
