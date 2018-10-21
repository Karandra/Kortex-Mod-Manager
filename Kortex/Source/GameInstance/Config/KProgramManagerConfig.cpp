#include "stdafx.h"
#include "KProgramManagerConfig.h"
#include "GameInstance/KGameInstance.h"
#include "KApp.h"
#include "KAux.h"

KProgramManagerConfig::KProgramManagerConfig(KGameInstance& profile, const KxXMLNode& rootNode)
{
	KProgramManager::GetInstance()->OnLoadConfig(rootNode);
}
