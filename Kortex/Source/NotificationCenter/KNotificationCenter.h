#pragma once
#include "stdafx.h"
#include "KNotification.h"
#include "KManager.h"
#include <KxFramework/KxSingleton.h>

class KNotificationCenter: public KxSingletonPtr<KNotificationCenter>
{
	friend class KNotificationPopup;

	private:
		KNotification::Containter m_Notifications;

	private:
		void DoNotify(KNotification* notification);

	public:
		KNotificationCenter();
		virtual ~KNotificationCenter();

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
