#pragma once
#include "Utility/UniqueID.h"

namespace Kortex::NetworkManager
{
	namespace Internal
	{
		enum ProviderIDs
		{
			Nexus,
			LoversLab,
			TESALL,

			MAX_SYSTEM,
			Invalid = -1
		};

		namespace Tags
		{
			struct ModID
			{
			};
			struct ModFileID
			{
			};
		}
	}
}

namespace Kortex
{
	using ModID = Utility::UniqueID::IntegerID<int64_t, -1, false, NetworkManager::Internal::Tags::ModID>;
	using ModFileID = Utility::UniqueID::IntegerID<int64_t, -1, false, NetworkManager::Internal::Tags::ModFileID>;
	
	using NetworkProviderID = intptr_t;
	using NetworkProviderIDs = NetworkManager::Internal::ProviderIDs;
}
