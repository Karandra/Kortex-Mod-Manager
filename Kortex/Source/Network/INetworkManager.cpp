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
		const INetworkProvider* provider = GetDefaultProvider();
		if (provider && provider->IsAuthenticated())
		{
			return true;
		}
		return false;
	}
	Network::ProviderID INetworkManager::GetDefaultProviderID() const
	{
		const INetworkProvider* provider = GetDefaultProvider();
		return provider ? provider->GetID() : Network::ProviderIDs::Invalid;
	}
}
