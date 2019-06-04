#include "stdafx.h"
#include "ModPackagesModule.h"
#include "PackageManager/DefaultPackageManager.h"
#include "Application/Resources/ImageResourceID.h"

namespace Kortex
{
	namespace Internal
	{
		const SimpleModuleInfo ModPackagesModuleTypeInfo("Packages", "PackagesModule.Name", "1.3.1", ImageResourceID::Box);
	}

	void ModPackagesModule::OnLoadInstance(IGameInstance& instance, const KxXMLNode& node)
	{
	}
	void ModPackagesModule::OnInit()
	{
	}
	void ModPackagesModule::OnExit()
	{
	}

	ModPackagesModule::ModPackagesModule()
		:ModuleWithTypeInfo(Disposition::Global)
	{
		m_PackageManager = std::make_unique<PackageManager::DefaultPackageManager>();
	}
}
