#pragma once
#include "stdafx.h"
#include "Application/IModule.h"
#include <KxFramework/KxSingleton.h>

namespace Kortex
{
	class IPackageManager;
}

namespace Kortex
{
	namespace Internal
	{
		extern const SimpleModuleInfo ModPackagesModuleTypeInfo;
	};

	class ModPackagesModule:
		public ModuleWithTypeInfo<IModule, Internal::ModPackagesModuleTypeInfo>,
		public KxSingletonPtr<ModPackagesModule>
	{
		private:
			std::unique_ptr<IPackageManager> m_PackageManager;

		protected:
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& node) override;
			void OnInit() override;
			void OnExit() override;

		public:
			ModPackagesModule();

		public:
			virtual ManagerRefVector GetManagers() override
			{
				return ToManagersList(m_PackageManager);
			}
	};
}