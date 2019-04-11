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

	bool INetworkManager::IsDefaultProviderAvailable() const
	{
		const INetworkModSource* provider = GetDefaultProvider();
		if (provider && provider->IsAuthenticated())
		{
			return true;
		}
		return false;
	}
	NetworkProviderID INetworkManager::GetDefaultProviderID() const
	{
		const INetworkModSource* provider = GetDefaultProvider();
		return provider ? provider->GetID() : NetworkProviderIDs::Invalid;
	}
}
