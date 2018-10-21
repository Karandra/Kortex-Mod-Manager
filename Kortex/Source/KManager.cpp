#include "stdafx.h"
#include "KPluggableManager.h"
#include "UI/KWorkspace.h"
#include "UI/KWorkspaceController.h"
#include "UI/KMainWindow.h"
#include "KApp.h"
#include "KAux.h"

namespace
{
	KManager::RefVector ms_Instances;
}

KManager::RefVector& KManager::GetActiveInstances()
{
	return ms_Instances;
}

KManager::KManager()
{
	ms_Instances.push_back(this);
}
KManager::~KManager()
{
	ms_Instances.erase(std::remove(ms_Instances.begin(), ms_Instances.end(), this), ms_Instances.end());
}
