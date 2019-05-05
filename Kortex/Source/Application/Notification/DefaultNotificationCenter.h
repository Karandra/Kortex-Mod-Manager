#pragma once
#include "stdafx.h"
#include "Application/INotification.h"
#include "Application/INotificationCenter.h"
class wxPopupTransientWindow;

namespace Kortex::Notification
{
	class DisplayModel;

	class DefaultNotificationCenter: public INotificationCenter
	{
		friend class KMainWindow;
		friend class PopupWindow;
		friend class DisplayModel;

		private:
			wxCriticalSection m_NotificationsCS;
			INotification::Vector m_Notifications;
			KxAuiToolBarItem* m_Button = nullptr;

			wxPopupTransientWindow* m_PopupWindow = nullptr;
			DisplayModel* m_PopupDisplayModel = nullptr;

		protected:
			void DoNotify(std::unique_ptr<INotification> notification) override;

			void OnSetToolBarButton(KxAuiToolBarItem* button) override;
			void OnToolBarButton(KxAuiToolBarEvent& event) override;
			void UpdateToolBarButton() override;

			const INotification::Vector& GetNotifications() const override
			{
				return m_Notifications;
			}
			INotification::Vector& GetNotifications() override
			{
				return m_Notifications;
			}

		public:
			bool HasActivePopups() const override;
			size_t GetActivePopupsCount() const override;
	};
}
