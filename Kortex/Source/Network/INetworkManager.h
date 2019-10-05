#pragma once
#include "stdafx.h"
#include "Application/IManager.h"
#include "Common.h"
#include "IModNetwork.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxURI.h>
class IMainWindow;
class KxAuiToolBarItem;
class KxAuiToolBarEvent;
class KxCURLSession;
class KxIWebSocketClient;

namespace Kortex
{
	class ModNetworkRepository;
}

namespace Kortex
{
	namespace NetworkManager
	{
		class Config;
	}
	namespace NetworkManager::Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	};

	class INetworkManager:
		public ManagerWithTypeInfo<IManager, NetworkManager::Internal::TypeInfo>,
		public Application::ManagerWithToolbarButton,
		public KxSingletonPtr<INetworkManager>
	{
		friend class IMainWindow;

		protected:
			enum class NetworkSoftware
			{
				LibCURL,
				WebSocket,
			};

		protected:
			wxString GetUserAgentString(NetworkSoftware networkSoftware) const;

			void OnInit() override;
			void OnExit() override;
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			
		public:
			INetworkManager();

		public:
			virtual wxString GetCacheDirectory() const = 0;
			
			virtual IModNetwork::RefVector GetModNetworks() = 0;
			std::vector<ModNetworkRepository*> GetModRepositories();

			virtual IModNetwork* GetDefaultModNetwork() const = 0;
			virtual IModNetwork* GetModNetworkByName(const wxString& name) const = 0;

			virtual void OnAuthStateChanged() = 0;

		public:
			virtual std::unique_ptr<KxIWebSocketClient> NewWebSocketClient(const KxURI& address);
			virtual std::unique_ptr<KxCURLSession> NewCURLSession(const KxURI& address);
			virtual std::unique_ptr<wxFileSystemHandler> NewWxFSHandler();
	};
}
