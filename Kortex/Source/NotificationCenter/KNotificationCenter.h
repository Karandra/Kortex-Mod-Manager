#pragma once
#include "stdafx.h"
#include "KNotification.h"
#include "KManager.h"
#include <KxFramework/KxSingleton.h>
class KNotificationPopupList;
class KxAuiToolBarItem;
class KxAuiToolBarEvent;
class wxPopupTransientWindow;

class KNotificationCenter: public KManager, public KxSingletonPtr<KNotificationCenter>
{
	friend class KMainWindow;
	friend class KNotificationPopup;
	friend class KNotificationPopupList;

	private:
		KNotification::Vector m_Notifications;
		KxAuiToolBarItem* m_Button = NULL;
		
		wxPopupTransientWindow* m_PopupListWindow = NULL;
		KNotificationPopupList* m_PopupListModel = NULL;

	private:
		void DoNotify(KNotification* notification);

		void OnSetToolBarButton(KxAuiToolBarItem* button);
		void OnToolBarButton(KxAuiToolBarEvent& event);
		void UpdateToolBarButton();

		const KNotification::Vector& GetNotifications() const
		{
			return m_Notifications;
		}
		KNotification::Vector& GetNotifications()
		{
			return m_Notifications;
		}

	public:
		KNotificationCenter();
		virtual ~KNotificationCenter();

	public:
		virtual wxString GetID() const;
		virtual wxString GetName() const;
		virtual wxString GetVersion() const;
		virtual KImageEnum GetImageID() const;

	public:
		void Notify(KNotification* notification)
		{
			DoNotify(notification);
		}
		void Notify(const wxString& caption, const wxString& message, KxIconType iconID = KxICON_INFORMATION)
		{
			DoNotify(new KNotificationSimple(caption, message, iconID));
		}
		void Notify(const wxString& caption, const wxString& message, const wxBitmap& bitmap = wxNullBitmap)
		{
			DoNotify(new KNotificationSimple(caption, message, bitmap));
		}
		void Notify(const KManager* manager, const wxString& message, KxIconType iconID = KxICON_INFORMATION)
		{
			DoNotify(new KNotificationSimple(manager->GetName(), message, iconID));
		}
		void Notify(const KManager* manager, const wxString& message, const wxBitmap& bitmap = wxNullBitmap)
		{
			DoNotify(new KNotificationSimple(manager->GetName(), message, bitmap));
		}

		bool HasActivePopups() const;
		size_t GetActivePopupsCount() const;
};
