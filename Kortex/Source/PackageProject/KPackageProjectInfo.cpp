#include "stdafx.h"
#include "KPackageProjectInfo.h"
#include "KPackageProject.h"
#include <Kortex/Application.hpp>
#include "KAux.h"
#include <Kortex/ModManager.hpp>

KPackageProjectInfo::KPackageProjectInfo(KPackageProject& project)
	:KPackageProjectPart(project)
{
}
KPackageProjectInfo::~KPackageProjectInfo()
{
}
