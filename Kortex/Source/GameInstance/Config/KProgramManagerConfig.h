#pragma once
#include "stdafx.h"
#include "ProgramManager/KProgramManager.h"
#include <KxFramework/KxSingleton.h>
class KGameInstance;

//////////////////////////////////////////////////////////////////////////
class KProgramManagerConfig: public KxSingletonPtr<KProgramManagerConfig>
{
	public:
		KProgramManagerConfig(KGameInstance& profile, const KxXMLNode& rootNode);
};
