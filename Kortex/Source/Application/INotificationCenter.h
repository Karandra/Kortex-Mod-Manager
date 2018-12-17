#pragma once
#include "stdafx.h"
#include "INotification.h"
#include <KxFramework/KxSingleton.h>
class KMainWindow;
class KxAuiToolBarItem;
class KxAuiToolBarEvent;

namespace Kortex
{
	namespace Notification
	{
		class DisplayModel;
	}

	class INotification;
	class IManager;
	class IModule;

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
			virtual void DoNotify(INotification* notification) = 0;

			virtual void OnSetToolBarButton(KxAuiToolBarItem* button) = 0;
			virtual void OnToolBarButton(KxAuiToolBarEvent& event) = 0;
			virtual void UpdateToolBarButton() = 0;

			virtual const INotification::Vector& GetNotifications() const = 0;
			virtual INotification::Vector& GetNotifications() = 0;

		public:
			INotificationCenter() = default;
			virtual ~INotificationCenter() = default;

		public:
			void Notify(INotification* notification)
			{
				DoNotify(notification);
			}
			void Notify(const wxString& caption, const wxString& message, KxIconType iconID = KxICON_INFORMATION);
			void Notify(const wxString& caption, const wxString& message, const wxBitmap& bitmap = wxNullBitmap);
			void Notify(const IManager* manager, const wxString& message, KxIconType iconID = KxICON_INFORMATION);
			void Notify(const IManager* manager, const wxString& message, const wxBitmap& bitmap = wxNullBitmap);
			template<class T, class... Args> void Notify(Args&&... arg)
			{
				DoNotify(new T(std::forward(arg)...));
			}

			virtual bool HasActivePopups() const = 0;
			virtual size_t GetActivePopupsCount() const = 0;
	};
}
