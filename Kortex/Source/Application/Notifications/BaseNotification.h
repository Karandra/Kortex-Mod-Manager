#pragma once
#include "stdafx.h"
#include "Application/INotification.h"
#include "KxFramework/KxUUID.h"

namespace Kortex::Notifications
{
	class TrayPopupHandler final
	{
		private:
			WNDCLASSEXW m_WindowClass = {};
			HWND m_Handle = nullptr;
			ATOM m_Atom = 0;

			int m_NotifyMessage = -1;
			int m_NotifyIconID = -1;

			NOTIFYICONDATAW m_NotifyInfo = {};
			KxUUID m_NotifyGUID;
			wxIcon m_NotifyIcon;
			wxIcon m_TrayIcon;

			bool m_IsDisplayed = false;
			bool m_SuppressDismiss = false;

		private:
			void OnNotificationEvent(int eventID, int iconID, const wxPoint& pos);
			void InitNotifyInfo();

			void InitMessages();
			void InitGUID();
			void CreateWindowClass();
			void CreateWindow();

		public:
			~TrayPopupHandler()
			{
				Destroy();
			}

		public:
			HWND GetHandle() const
			{
				return m_Handle;
			}
			KxUUID GetGUID() const
			{
				return m_NotifyGUID;
			}
			int GetMessage() const
			{
				return m_NotifyMessage;
			}
			int GetIconID() const
			{
				return m_NotifyIconID;
			}

			bool IsDisplayed() const
			{
				return m_IsDisplayed;
			}
			void Popup(const INotification& notification);
			void Dismiss();

			void Create();
			void Destroy();
	};
}

namespace Kortex::Notifications
{
	class BaseNotification: public INotification
	{
		friend class DefaultNotificationCenter;

		private:
			std::unique_ptr<INotificationPopup> m_Popup;

		private:
			void DoDestroy();

		public:
			BaseNotification() = default;
			~BaseNotification()
			{
				DoDestroy();
			}

		public:
			void Popup() override;
			bool HasPopup() const override;
			void DestroyPopup() override;
	};
}
