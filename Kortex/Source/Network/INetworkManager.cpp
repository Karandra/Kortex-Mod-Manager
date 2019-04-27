#include "stdafx.h"
#include "INetworkManager.h"
#include "NetworkWxFSHandler.h"
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

	INetworkManager::INetworkManager()
		:ManagerWithTypeInfo(NetworkModule::GetInstance())
	{
	}

	std::unique_ptr<KxWebSocket::IClient> INetworkManager::NewWebSocketClient(const wxString& address)
	{
		auto webSocket = KxWebSocket::NewSecureClient(address);
		webSocket->SetUserAgent(GetUserAgentString(NetworkSoftware::WebSocket));
		AddDefaultHeaders(*webSocket);

		return webSocket;
	}
	std::unique_ptr<KxCURLSession> INetworkManager::NewCURLSession(const wxString& address)
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
