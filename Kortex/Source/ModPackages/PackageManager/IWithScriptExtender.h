#pragma once
#include "stdafx.h"
#include "ModPackages/IPackageManager.h"
#include <KxFramework/KxComponentSystem.h>

namespace Kortex::PackageDesigner
{
	class KPPRRequirementEntry;
}

namespace Kortex::PackageDesigner
{
	class IWithScriptExtender: public KxComponentOf<IPackageManager>
	{
		public:
			virtual const KPPRRequirementEntry& GetEntry() const = 0;
	};
}
