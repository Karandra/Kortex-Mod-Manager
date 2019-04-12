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

	bool INetworkManager::IsDefaultProviderAuthenticated() const
	{
		const IModSource* modSource = GetDefaultModSource();
		if (modSource && modSource->IsAuthenticated())
		{
			return true;
		}
		return false;
	}
}
