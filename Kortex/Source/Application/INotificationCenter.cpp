#include "stdafx.h"
#include "INotificationCenter.h"
#include <Kortex/Application.hpp>
#include <Kortex/Notification.hpp>
#include <Kortex/NetworkManager.hpp>

namespace Kortex::Notifications::Internal
{
	const SimpleManagerInfo TypeInfo("NotificationCenter", "NotificationCenter.Name");
}

namespace Kortex
{
	void INotificationCenter::OnInit()
	{
	}
	void INotificationCenter::OnExit()
	{
		for (auto& notification: GetNotifications())
		{
			notification->DestroyPopupWindow();
		}
	}
	void INotificationCenter::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
	}

	INotificationCenter::INotificationCenter()
		:ManagerWithTypeInfo(&IApplication::GetInstance()->GetModule())
	{
	}

	void INotificationCenter::Notify(const wxString& caption, const wxString& message, KxIconType iconID)
	{
		DoNotify(std::make_unique<SimpleNotification>(caption, message, iconID));
	}
	void INotificationCenter::Notify(const wxString& caption, const wxString& message, const wxBitmap& bitmap)
	{
		DoNotify(std::make_unique<SimpleNotification>(caption, message, bitmap));
	}
	
	void INotificationCenter::Notify(const IModule& module, const wxString& message, KxIconType iconID)
	{
		DoNotify(std::make_unique<SimpleNotification>(module.GetModuleInfo().GetName(), message, iconID));
	}
	void INotificationCenter::Notify(const IModule& module, const wxString& message, const wxBitmap& bitmap)
	{
		DoNotify(std::make_unique<SimpleNotification>(module.GetModuleInfo().GetName(), message, bitmap));
	}

	void INotificationCenter::Notify(const IManager& manager, const wxString& message, KxIconType iconID)
	{
		DoNotify(std::make_unique<SimpleNotification>(manager.GetManagerInfo().GetName(), message, iconID));
	}
	void INotificationCenter::Notify(const IManager& manager, const wxString& message, const wxBitmap& bitmap)
	{
		DoNotify(std::make_unique<SimpleNotification>(manager.GetManagerInfo().GetName(), message, bitmap));
	}

	void INotificationCenter::Notify(const IModNetwork& modNetwork, const wxString& message, KxIconType iconID)
	{
		DoNotify(std::make_unique<SimpleNotification>(modNetwork.GetName(), message, iconID));
	}
	void INotificationCenter::Notify(const IModNetwork& modNetwork, const wxString& message, const wxBitmap& bitmap)
	{
		DoNotify(std::make_unique<SimpleNotification>(modNetwork.GetName(), message, bitmap));
	}
}
