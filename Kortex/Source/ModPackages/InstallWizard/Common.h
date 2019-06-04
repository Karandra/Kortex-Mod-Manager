#pragma once
#include "stdafx.h"
#include "PackageProject/KPackageProject.h"

namespace Kortex::InstallWizard
{
	using TImagesMap = std::unordered_map<int, const KPPIImageEntry*>;
	using TFlagsMap = std::unordered_map<wxString, wxString>;
}
