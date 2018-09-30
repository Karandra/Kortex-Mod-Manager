#pragma once

using KNetworkModID = int64_t;
using KNetworkFileID = int64_t;

enum KNetworkProviderID: intptr_t
{
	KNETWORK_PROVIDER_ID_INVALID = -1,

	KNETWORK_PROVIDER_ID_NEXUS,
	KNETWORK_PROVIDER_ID_TESALL,
	KNETWORK_PROVIDER_ID_LOVERSLAB,

	KNETWORK_PROVIDER_ID_MAX,
	KNETWORK_PROVIDER_ID_FIRST = KNETWORK_PROVIDER_ID_NEXUS,
};
