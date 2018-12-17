#pragma once

namespace Kortex::Network
{
	namespace Internal
	{
		enum ProviderIDs
		{
			Nexus,
			TESALL,
			LoversLab,

			MAX_SYSTEM,
			Invalid = -1
		};
	}

	using ModID = int64_t;
	using FileID = int64_t;
	using ProviderID = int64_t;

	using ProviderIDs = Internal::ProviderIDs;
	enum: ModID
	{
		InvalidModID = -1,
	};
}
