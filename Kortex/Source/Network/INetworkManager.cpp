#include "stdafx.h"
#include "INetworkManager.h"
#include <Kortex/NetworkManager.hpp>

namespace Kortex
{
	namespace NetworkManager::Internal
	{
		const SimpleManagerInfo TypeInfo("NetworkManager", "Network.Name");
	}

	INetworkManager::INetworkManager()
		:ManagerWithTypeInfo(NetworkModule::GetInstance())
	{
	}

	bool INetworkManager::IsDefaultModSourceAuthenticated() const
	{
		const IModSource* modSource = GetDefaultModSource();
		const IAuthenticableModSource* auth = nullptr;
		
		return modSource && modSource->QueryInterface(auth) && auth->IsAuthenticated();
	}
}
