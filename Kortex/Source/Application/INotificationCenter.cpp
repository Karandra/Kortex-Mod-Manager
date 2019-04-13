#include "stdafx.h"
#include "INotificationCenter.h"
#include <Kortex/Application.hpp>
#include <Kortex/Notification.hpp>
#include <Kortex/NetworkManager.hpp>

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
	
	void INotificationCenter::Notify(const IModule& module, const wxString& message, KxIconType iconID)
	{
		DoNotify(new SimpleNotification(module.GetModuleInfo().GetName(), message, iconID));
	}
	void INotificationCenter::Notify(const IModule& module, const wxString& message, const wxBitmap& bitmap)
	{
		DoNotify(new SimpleNotification(module.GetModuleInfo().GetName(), message, bitmap));
	}

	void INotificationCenter::Notify(const IManager& manager, const wxString& message, KxIconType iconID)
	{
		DoNotify(new SimpleNotification(manager.GetManagerInfo().GetName(), message, iconID));
	}
	void INotificationCenter::Notify(const IManager& manager, const wxString& message, const wxBitmap& bitmap)
	{
		DoNotify(new SimpleNotification(manager.GetManagerInfo().GetName(), message, bitmap));
	}

	void INotificationCenter::Notify(const IModSource& modSource, const wxString& message, KxIconType iconID)
	{
		DoNotify(new SimpleNotification(modSource.GetName(), message, iconID));
	}
	void INotificationCenter::Notify(const IModSource& modSource, const wxString& message, const wxBitmap& bitmap)
	{
		DoNotify(new SimpleNotification(modSource.GetName(), message, bitmap));
	}
}
