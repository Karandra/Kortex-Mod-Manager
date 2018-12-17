#include "stdafx.h"
#include <Kortex/SaveManager.hpp>
#include <Kortex/GameInstance.hpp>
#include "ISaveManager.h"

namespace Kortex
{
	namespace SaveManager::Internal
	{
		const SimpleManagerInfo TypeInfo("SaveManager", "SaveManager.Name");
	}

	ISaveManager::ISaveManager()
		:ManagerWithTypeInfo(GameDataModule::GetInstance())
	{
	}
}
