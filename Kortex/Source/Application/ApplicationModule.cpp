#include "stdafx.h"
#include "ApplicationModule.h"
#include "SystemApplication.h"
#include "INotificationCenter.h"
#include "Resources/ImageResourceID.h"

namespace Kortex::Application::Internal
{
	wxString ApplicationModuleInfo::GetID() const
	{
		return SystemApplication::GetInstance()->GetAppName();
	}
	wxString ApplicationModuleInfo::GetName() const
	{
		return SystemApplication::GetInstance()->GetAppDisplayName();
	}
	KxVersion ApplicationModuleInfo::GetVersion() const
	{
		return SystemApplication::GetInstance()->GetAppVersion();
	}
	ResourceID ApplicationModuleInfo::GetImageID() const
	{
		return ImageResourceID::KortexLogoSmall;
	}
}

namespace Kortex::Application
{
	void ApplicationModule::OnLoadInstance(IGameInstance& instance, const KxXMLNode& node)
	{
	}
	void ApplicationModule::OnInit()
	{
	}
	void ApplicationModule::OnExit()
	{
	}

	ApplicationModule::ApplicationModule()
		:IModule(Disposition::Global)
	{
	}

	IModule::ManagerRefVector ApplicationModule::GetManagers()
	{
		return {INotificationCenter::GetInstance()};
	}
}
