#include "stdafx.h"
#include "NetworkModule.h"
#include "NetworkManager/DefaultNetworkManager.h"
#include "DownloadManager/DefaultDownloadManager.h"
#include "KINetFSHandler.h"

namespace Kortex
{
	namespace Internal
	{
		const SimpleModuleInfo NetworkModuleTypeInfo("Network", "NetworkModule.Name", "1.0", KIMG_SITE_UNKNOWN);
	}

	void NetworkModule::OnInit()
	{
	}
	void NetworkModule::OnExit()
	{
	}
	void NetworkModule::OnLoadInstance(IGameInstance& instance, const KxXMLNode& node)
	{
		m_NetworkManager = std::make_unique<NetworkManager::DefaultNetworkManager>();
		m_DownloadManager = CreateManagerIfEnabled<DownloadManager::DefaultDownloadManager>(node);

		wxFileSystem::AddHandler(new KINetFSHandler());
	}

	NetworkModule::NetworkModule()
		:ModuleWithTypeInfo(Disposition::Global)
	{
	}

	IModule::ManagerRefVector NetworkModule::GetManagers()
	{
		return ToManagersList(m_NetworkManager, m_DownloadManager);
	}
}
