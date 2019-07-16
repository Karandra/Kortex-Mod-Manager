#include "stdafx.h"
#include "INetworkManager.h"
#include "NetworkWxFSHandler.h"
#include "ModNetworkRepository.h"
#include <Kortex/NetworkManager.hpp>
#include <Kortex/Application.hpp>
#include <KxFramework/KxWebSocket.h>
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxSystem.h>

namespace
{
	template<class T> void AddDefaultHeaders(T& object)
	{
		using namespace Kortex;
		const IApplication* application = IApplication::GetInstance();

		object.AddHeader(wxS("Application-Name"), application->GetShortName());
		object.AddHeader(wxS("Application-Version"), application->GetVersion());
	}
}

namespace Kortex
{
	namespace NetworkManager::Internal
	{
		const SimpleManagerInfo TypeInfo("NetworkManager", "NetworkManager.Name");
	}

	wxString INetworkManager::GetUserAgentString(NetworkSoftware networkSoftware) const
	{
		const IApplication* application = IApplication::GetInstance();
		KxFormat formatter(wxS("%1/%2 (Windows_NT %3; %4) %5/%6"));

		// Application name and version
		formatter(application->GetShortName());
		formatter(application->GetVersion());

		// Windows version
		auto versionInfo = KxSystem::GetKernelVersion();
		formatter(KxString::Format(wxS("%1.%2.%3"), versionInfo.Major, versionInfo.Minor, versionInfo.Build));

		// System architecture (x86/x64)
		formatter(KVarExp(wxS("$(SystemArchitectureName)")));

		// Network software
		switch (networkSoftware)
		{
			case NetworkSoftware::CURL:
			{
				formatter(KxCURL::GetLibraryName());
				formatter(KxCURL::GetLibraryVersion());
				break;
			}
			case NetworkSoftware::WebSocket:
			{
				formatter(KxWebSocket::GetLibraryName());
				formatter(KxWebSocket::GetLibraryVersion());
				break;
			}
			default:
			{
				formatter(wxS("Unknown"));
				formatter(wxS("0.0"));
			}
		};

		return formatter;
	}

	void INetworkManager::OnInit()
	{
		for (auto& modNetwork: GetModNetworks())
		{
			modNetwork->DoOnInit();
		}
	}
	void INetworkManager::OnExit()
	{
		for (auto& modNetwork: GetModNetworks())
		{
			modNetwork->DoOnExit();
		}
	}
	void INetworkManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		std::unordered_set<IModNetwork*> initialized;

		for (KxXMLNode node = managerNode.GetFirstChildElement(wxS("ModNetwork")); node.IsOK(); node = node.GetNextSiblingElement(wxS("ModNetwork")))
		{
			IModNetwork* modNetwork = GetModNetworkByName(node.GetAttribute(wxS("Name")));

			// Initialize mod network only once even if there are multiple configurations for the same network
			if (modNetwork && initialized.count(modNetwork) == 0)
			{
				modNetwork->OnLoadInstance(instance, node);
				initialized.insert(modNetwork);
			}
		}

		// Call 'OnLoadInstance' with invalid node for every not already loaded mod network
		for (IModNetwork* modNetwork: GetModNetworks())
		{
			if (initialized.count(modNetwork) == 0)
			{
				modNetwork->OnLoadInstance(instance, {});
			}
		}
	}

	INetworkManager::INetworkManager()
		:ManagerWithTypeInfo(NetworkModule::GetInstance())
	{
	}

	std::vector<ModNetworkRepository*> INetworkManager::GetModRepositories()
	{
		std::vector<ModNetworkRepository*> reposirories;
		for (IModNetwork* modNetwork: GetModNetworks())
		{
			ModNetworkRepository* repository = nullptr;
			if (modNetwork->TryGetComponent(repository))
			{
				reposirories.push_back(repository);
			}
		}
		return reposirories;
	}

	std::unique_ptr<KxIWebSocketClient> INetworkManager::NewWebSocketClient(const KxURI& address)
	{
		auto webSocket = KxWebSocket::NewSecureClient(address);
		webSocket->SetUserAgent(GetUserAgentString(NetworkSoftware::WebSocket));
		AddDefaultHeaders(*webSocket);

		return webSocket;
	}
	std::unique_ptr<KxCURLSession> INetworkManager::NewCURLSession(const KxURI& address)
	{
		auto curlSession = std::make_unique<KxCURLSession>(address);
		curlSession->SetUserAgent(GetUserAgentString(NetworkSoftware::CURL));
		AddDefaultHeaders(*curlSession);

		return curlSession;
	}
	std::unique_ptr<wxFileSystemHandler> INetworkManager::NewWxFSHandler()
	{
		return std::make_unique<NetworkManager::NetworkWxFSHandler>(*this);
	}
}
