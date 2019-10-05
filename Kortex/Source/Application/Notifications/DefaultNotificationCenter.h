#pragma once
#include "stdafx.h"
#include "Application/INotification.h"
#include "Application/INotificationCenter.h"
class wxPopupTransientWindow;

namespace Kortex::Notifications
{
	class DisplayModel;

	class DefaultNotificationCenter: public INotificationCenter
	{
		friend class IMainWindow;
		friend class PopupWindow;
		friend class DisplayModel;

		private:
			wxCriticalSection m_NotificationsCS;
			INotification::Vector m_Notifications;
			KxAuiToolBarItem* m_Button = nullptr;

			wxPopupTransientWindow* m_PopupWindow = nullptr;
			KxAuiToolBar* m_PpoupToolbar = nullptr;
			KxAuiToolBarItem* m_PpoupToolbar_Label = nullptr;
			KxAuiToolBarItem* m_PpoupToolbar_ClearNotifications = nullptr;
			DisplayModel* m_PopupDisplayModel = nullptr;

		private:
			void UpdateLabel();
			void OnClearNotifications(KxAuiToolBarEvent& event);
			void OnNotificationsCountChanged();

		protected:
			void OnSetToolbarButton(KxAuiToolBarItem& button) override;
			void OnToolbarButton(KxAuiToolBarEvent& event) override;
			void UpdateToolbarButton() override;

			void QueueNotification(std::unique_ptr<INotification> notification) override;
			void OnNotificationAdded(INotification& notification) override
			{
				OnNotificationsCountChanged();
			}
			void OnNotificationRemoved(INotification& notification) override
			{
				OnNotificationsCountChanged();
			}
			void OnNotificationsCleared() override
			{
				OnNotificationsCountChanged();
			}

			INotification::Vector& GetNotifications() override
			{
				return m_Notifications;
			}

		public:
			bool IsNotificationsDisplayed() const override;
			void ShowNotificationsWindow() override;
			void HideNotificationsWindow() override;
	};
}
