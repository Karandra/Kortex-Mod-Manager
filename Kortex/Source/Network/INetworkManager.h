#pragma once
#include "stdafx.h"
#include "Application/IManager.h"
#include "Common.h"
#include "IModNetwork.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxCURL.h>
class KMainWindow;
class KxAuiToolBarItem;
class KxAuiToolBarEvent;
class KxCURLSession;

namespace KxWebSocket
{
	class IClient;
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
			
		public:
			INetworkManager();

		public:
			virtual const NetworkManager::Config& GetConfig() const = 0;
			virtual wxString GetCacheFolder() const = 0;
			
			virtual IModNetwork::Vector& GetModNetworks() = 0;
			virtual IModNetwork* GetDefaultModNetwork() const = 0;
			virtual IModNetwork* GetModNetworkByName(const wxString& name) const = 0;

			virtual void OnAuthStateChanged() = 0;

		public:
			virtual std::unique_ptr<KxWebSocket::IClient> NewWebSocketClient(const wxString& address);
			virtual std::unique_ptr<KxCURLSession> NewCURLSession(const wxString& address);
			virtual std::unique_ptr<wxFileSystemHandler> NewWxFSHandler();
	};
}
