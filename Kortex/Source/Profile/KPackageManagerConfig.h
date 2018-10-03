#pragma once
#include "stdafx.h"
#include <KxFramework/KxSingleton.h>
class KProfile;
class KPackageManager;

class KPackageManagerConfig: public KxSingletonPtr<KPackageManagerConfig>
{
	public:
		KPackageManagerConfig(KProfile& profile, const KxXMLNode& node);
		~KPackageManagerConfig();
};
