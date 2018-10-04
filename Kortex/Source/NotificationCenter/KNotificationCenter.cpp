#include "stdafx.h"
#include "KNotificationCenter.h"
#include "KNotificationPopup.h"
#include <KxFramework/KxCoroutine.h>

void KNotificationCenter::DoNotify(KNotification* notification)
{
	m_Notifications.emplace(m_Notifications.begin(), notification);
	notification->ShowPopupWindow();
}

KNotificationCenter::KNotificationCenter()
{
}
KNotificationCenter::~KNotificationCenter()
{
}

bool KNotificationCenter::HasActivePopups() const
{
	for (const auto& notification: m_Notifications)
	{
		if (notification->HasPopupWindow())
		{
			return true;
		}
	}
	return false;
}
size_t KNotificationCenter::GetActivePopupsCount() const
{
	size_t count = 0;
	for (const auto& notification: m_Notifications)
	{
		if (notification->HasPopupWindow())
		{
			count++;
		}
	}
	return count;
}
