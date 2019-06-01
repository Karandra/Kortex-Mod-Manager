#pragma once
#include "stdafx.h"
#include "Module/ModuleInfo.h"

namespace Kortex
{
	class SystemApplication;
}

namespace Kortex::Application::Internal
{
	extern const SimpleModuleInfo ApplicationModuleTypeInfo;

	class ApplicationModuleInfo: public IModuleInfo
	{
		public:
			wxString GetID() const override;
			wxString GetName() const override;
			KxVersion GetVersion() const override;
			ResourceID GetImageID() const override;
	};
}

namespace Kortex::Application
{
	class ApplicationModule: public IModule
	{
		friend class SystemApplication;

		private:
			Internal::ApplicationModuleInfo m_ModuleInfo;

		protected:
			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& node) override;
			virtual void OnInit() override;
			virtual void OnExit() override;

		private:
			ApplicationModule();

		public:
			const IModuleInfo& GetModuleInfo() const override
			{
				return m_ModuleInfo;
			}
			ManagerRefVector GetManagers() override;
	};
}
