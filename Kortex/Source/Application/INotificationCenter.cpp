#include "stdafx.h"
#include "INotificationCenter.h"
#include <Kortex/Application.hpp>
#include <Kortex/Notification.hpp>

namespace Kortex
{
	void INotificationCenter::Notify(const wxString& caption, const wxString& message, KxIconType iconID)
	{
		DoNotify(new SimpleNotification(caption, message, iconID));
	}
	void INotificationCenter::Notify(const wxString& caption, const wxString& message, const wxBitmap& bitmap)
	{
		DoNotify(new SimpleNotification(caption, message, bitmap));
	}
	void INotificationCenter::Notify(const IManager* manager, const wxString& message, KxIconType iconID)
	{
		DoNotify(new SimpleNotification(manager->GetManagerInfo().GetName(), message, iconID));
	}
	void INotificationCenter::Notify(const IManager* manager, const wxString& message, const wxBitmap& bitmap)
	{
		DoNotify(new SimpleNotification(manager->GetManagerInfo().GetName(), message, bitmap));
	}
}
