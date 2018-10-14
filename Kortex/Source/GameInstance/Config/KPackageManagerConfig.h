#pragma once
#include "stdafx.h"
#include <KxFramework/KxSingleton.h>
class KGameInstance;
class KPackageManager;

class KPackageManagerConfig: public KxSingletonPtr<KPackageManagerConfig>
{
	public:
		KPackageManagerConfig(KGameInstance& profile, const KxXMLNode& node);
		~KPackageManagerConfig();
};
