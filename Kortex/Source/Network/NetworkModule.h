#pragma once
#include "stdafx.h"
#include "Application/IModule.h"
#include "Common.h"
#include "INetworkProvider.h"
#include <KxFramework/KxSingleton.h>
class KMainWindow;
class KxAuiToolBarEvent;
class KxAuiToolBarItem;
class KxMenuEvent;
class KxMenu;

namespace Kortex
{
	namespace Internal
	{
		extern const SimpleModuleInfo NetworkModuleTypeInfo;
	};

	class INetworkProvider;
	class INetworkManager;
	class IDownloadManager;

	class NetworkModule:
		public ModuleWithTypeInfo<IModule, SimpleModuleInfo, Internal::NetworkModuleTypeInfo>,
		public KxSingletonPtr<NetworkModule>
	{
		friend class KMainWindow;

		private:
			std::unique_ptr<INetworkManager> m_NetworkManager;
			std::unique_ptr<IDownloadManager> m_DownloadManager;

		private:
			virtual void OnInit() override;
			virtual void OnExit() override;
			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& node) override;

		public:
			NetworkModule();

		public:
			ManagerRefVector GetManagers() override;
	};
}
