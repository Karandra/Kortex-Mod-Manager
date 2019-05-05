#pragma once
#include "stdafx.h"
#include "INotification.h"
#include <KxFramework/KxSingleton.h>
class KMainWindow;
class KxAuiToolBarItem;
class KxAuiToolBarEvent;

namespace Kortex
{
	class INotification;
	class IManager;
	class IModule;
	class IModNetwork;
}
namespace Kortex::Notification
{
	class DisplayModel;
}

namespace Kortex
{
	class INotificationCenter: public KxSingletonPtr<INotificationCenter>
	{
		friend class KMainWindow;
		friend class Notification::DisplayModel;

		private:
			void CallOnToolBarButton(KxAuiToolBarEvent& event)
			{
				OnToolBarButton(event);
			}

		protected:
			virtual void DoNotify(std::unique_ptr<INotification> notification) = 0;

			virtual void OnSetToolBarButton(KxAuiToolBarItem* button) = 0;
			virtual void OnToolBarButton(KxAuiToolBarEvent& event) = 0;
			virtual void UpdateToolBarButton() = 0;

			virtual const INotification::Vector& GetNotifications() const = 0;
			virtual INotification::Vector& GetNotifications() = 0;

		public:
			INotificationCenter() = default;
			virtual ~INotificationCenter() = default;

		public:
			virtual bool HasActivePopups() const = 0;
			virtual size_t GetActivePopupsCount() const = 0;

			virtual void ShowNotificationsWindow() = 0;
			virtual void HideNotificationsWindow() = 0;

		public:
			void Notify(std::unique_ptr<INotification> notification)
			{
				DoNotify(std::move(notification));
			}
			
			void Notify(const wxString& caption, const wxString& message, KxIconType iconID = KxICON_INFORMATION);
			void Notify(const wxString& caption, const wxString& message, const wxBitmap& bitmap = wxNullBitmap);

			void Notify(const IModule& module, const wxString& message, KxIconType iconID = KxICON_INFORMATION);
			void Notify(const IModule& module, const wxString& message, const wxBitmap& bitmap = wxNullBitmap);

			void Notify(const IManager& manager, const wxString& message, KxIconType iconID = KxICON_INFORMATION);
			void Notify(const IManager& manager, const wxString& message, const wxBitmap& bitmap = wxNullBitmap);

			void Notify(const IModNetwork& modNetwork, const wxString& message, KxIconType iconID = KxICON_INFORMATION);
			void Notify(const IModNetwork& modNetwork, const wxString& message, const wxBitmap& bitmap = wxNullBitmap);
			
			template<class T, class... Args> void Notify(Args&&... arg)
			{
				DoNotify(std::make_unique<T>(std::forward<Args>(arg)...));
			}
			template<class TSingletonPtr, class... Args> void NotifyUsing(Args&&... arg)
			{
				Notify(*TSingletonPtr::GetInstance(), std::forward<Args>(arg)...);
			}
	};
}
