#include "stdafx.h"
#include "IConfigManager.h"
#include <Kortex/Common/GameConfig.hpp>

namespace Kortex
{
	namespace ConfigManager::Internal
	{
		const SimpleManagerInfo TypeInfo("ConfigManager", "ConfigManager.Name");
	}

	IConfigManager::IConfigManager()
		:ManagerWithTypeInfo(GameConfigModule::GetInstance())
	{
	}
}
