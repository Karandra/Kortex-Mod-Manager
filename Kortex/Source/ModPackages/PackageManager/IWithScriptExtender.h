#pragma once
#include "stdafx.h"
#include "ModPackages/IPackageManager.h"
#include <KxFramework/KxComponentSystem.h>

namespace Kortex::PackageProject
{
	class RequirementItem;
}

namespace Kortex::PackageDesigner
{
	class IWithScriptExtender: public KxComponentOf<IPackageManager>
	{
		public:
			virtual const PackageProject::RequirementItem& GetEntry() const = 0;
	};
}
