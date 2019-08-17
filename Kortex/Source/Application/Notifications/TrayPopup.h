#pragma once
#include "stdafx.h"
#include "Application/INotification.h"
#include "KxFramework/KxUUID.h"

namespace Kortex::Notifications
{
	class TrayPopup;

	class TrayPopupHandler final
	{
		private:
			static LRESULT CALLBACK WindowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

		private:
			TrayPopup& m_Popup;

			WNDCLASSEXW m_WindowClass = {};
			HWND m_Handle = nullptr;
			ATOM m_Atom = 0;

		public:
			TrayPopupHandler(TrayPopup& popup);
			~TrayPopupHandler();

		public:
			HWND GetHandle() const
			{
				return m_Handle;
			}
			void Destroy();
	};
}

namespace Kortex::Notifications
{
	class TrayPopup: public INotificationPopup
	{
		friend class TrayPopupHandler;

		private:
			INotification* m_Notification = nullptr;
			TrayPopupHandler m_Handler;

			NOTIFYICONDATAW m_Info = {};
			KxUUID m_GUID;
			wxIcon m_Icon;
			wxIcon m_TrayIcon;

		private:
			void CreateUI();
			void DoDestroy();
			void DoDismiss();

			void OnBallonClick();
			void OnBallonTimeout();
			void OnBallonShow();
			void OnBallonHide();

		public:
			TrayPopup(INotification& notification, const KxUUID& guid);
			~TrayPopup();

		public:
			void Popup() override;
			void Dismiss() override;
			void Destroy() override;
	};
}
