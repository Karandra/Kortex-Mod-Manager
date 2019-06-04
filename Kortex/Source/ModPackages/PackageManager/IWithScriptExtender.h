#pragma once
#include "stdafx.h"
#include "ModPackages/IPackageManager.h"
#include <KxFramework/KxComponentSystem.h>
class KPPRRequirementEntry;

namespace Kortex::PackageManager
{
	class IWithScriptExtender: public KxComponentOf<IPackageManager>
	{
		public:
			virtual const KPPRRequirementEntry& GetEntry() const = 0;
	};
}
