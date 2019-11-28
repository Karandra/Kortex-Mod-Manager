#pragma once
#include "stdafx.h"
#include "ModPackages/IPackageManager.h"
#include <KxFramework/KxComponentSystem.h>

namespace Kortex::PackageProject
{
	class KPPRRequirementEntry;
}

namespace Kortex::PackageDesigner
{
	class IWithScriptExtender: public KxComponentOf<IPackageManager>
	{
		public:
			virtual const PackageProject::KPPRRequirementEntry& GetEntry() const = 0;
	};
}
