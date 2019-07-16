#pragma once
#include "stdafx.h"
#include "Application/IManager.h"
#include "Common.h"
#include "IModNetwork.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxURI.h>
class KMainWindow;
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
		public KxSingletonPtr<INetworkManager>
	{
		friend class KMainWindow;

		protected:
			enum class NetworkSoftware
			{
				CURL,
				WebSocket,
			};

		private:
			void CallOnToolBarButton(KxAuiToolBarEvent& event)
			{
				OnToolBarButton(event);
			}

		protected:
			virtual void OnSetToolBarButton(KxAuiToolBarItem* button) = 0;
			virtual void OnToolBarButton(KxAuiToolBarEvent& event) = 0;

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
