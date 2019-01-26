#pragma once
#include "stdafx.h"
#include "Application/IModule.h"
#include <KxFramework/KxSingleton.h>

namespace Kortex
{
	namespace Internal
	{
		extern const SimpleModuleInfo GameConfigModuleTypeInfo;
	};

	class GameConfigModule:
		public ModuleWithTypeInfo<IModule, Internal::GameConfigModuleTypeInfo>,
		public KxSingletonPtr<GameConfigModule>
	{
		friend class KMainWindow;

		private:
			//std::unique_ptr<INetworkManager> m_NetworkManager;

		private:
			virtual void OnInit() override;
			virtual void OnExit() override;
			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& node) override;

		public:
			GameConfigModule();

		public:
			ManagerRefVector GetManagers() override;
	};
}
