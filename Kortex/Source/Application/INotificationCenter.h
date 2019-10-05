#pragma once
#include "stdafx.h"
#include "INotification.h"
#include "IManager.h"
#include <KxFramework/KxSingleton.h>
class IMainWindow;
class KxAuiToolBar;
class KxAuiToolBarItem;
class KxAuiToolBarEvent;

namespace Kortex
{
	class INotification;
	class IManager;
	class IModule;
	class IModNetwork;
}
namespace Kortex::Notifications
{
	class DisplayModel;

	namespace Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	}
}

namespace Kortex
{
	class INotificationCenter:
		public ManagerWithTypeInfo<IManager, Notifications::Internal::TypeInfo>,
		public Application::ManagerWithToolbarButton,
		public KxSingletonPtr<INotificationCenter>
	{
		friend class IMainWindow;
		friend class Notifications::DisplayModel;

		protected:
			void OnInit() override;
			void OnExit() override;
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;

			virtual void QueueNotification(std::unique_ptr<INotification> notification) = 0;
			virtual void OnNotificationAdded(INotification& notification) { }
			virtual void OnNotificationRemoved(INotification& notification) { }
			virtual void OnNotificationsCleared() { }

			virtual INotification::Vector& GetNotifications() = 0;
			const INotification::Vector& GetNotifications() const
			{
				return const_cast<INotificationCenter*>(this)->GetNotifications();
			}

		public:
			INotificationCenter();
			virtual ~INotificationCenter() = default;

		public:
			bool HasActivePopups() const;
			size_t GetActivePopupsCount() const;

			virtual bool IsNotificationsDisplayed() const = 0;
			virtual void ShowNotificationsWindow() = 0;
			virtual void HideNotificationsWindow() = 0;
			
			bool RemoveNotification(INotification& notification);
			bool ClearNotifications();

		public:
			void Notify(std::unique_ptr<INotification> notification)
			{
				QueueNotification(std::move(notification));
			}
			
			static void Notify(const wxString& caption, const wxString& message, KxIconType iconID = KxICON_INFORMATION);
			static void Notify(const wxString& caption, const wxString& message, const wxBitmap& bitmap = wxNullBitmap);

			static void Notify(const IModule& module, const wxString& message, KxIconType iconID = KxICON_INFORMATION);
			static void Notify(const IModule& module, const wxString& message, const wxBitmap& bitmap = wxNullBitmap);

			static void Notify(const IManager& manager, const wxString& message, KxIconType iconID = KxICON_INFORMATION);
			static void Notify(const IManager& manager, const wxString& message, const wxBitmap& bitmap = wxNullBitmap);

			static void Notify(const IModNetwork& modNetwork, const wxString& message, KxIconType iconID = KxICON_INFORMATION);
			static void Notify(const IModNetwork& modNetwork, const wxString& message, const wxBitmap& bitmap = wxNullBitmap);
			
			template<class T, class... Args> static void Notify(Args&&... arg)
			{
				GetInstance()->DoNotify(std::make_unique<T>(std::forward<Args>(arg)...));
			}
			template<class TSingletonPtr, class... Args> static void NotifyUsing(Args&&... arg)
			{
				Notify(*TSingletonPtr::GetInstance(), std::forward<Args>(arg)...);
			}
	};
}
