#include "stdafx.h"
#include "IModManager.h"
#include "GameMods/GameModsModule.h"

namespace Kortex
{
	namespace ModManager::Internal
	{
		const SimpleManagerInfo TypeInfo("ModManager", "ModManager.Name");
	}

	intptr_t IModManager::GetOrderIndex(const IGameMod& mod) const
	{
		const IGameMod::Vector& mods = GetMods();
		auto it = std::find_if(mods.begin(), mods.end(), [&mod](const auto& currentMod)
		{
			return currentMod.get() == &mod;
		});
		if (it != mods.end())
		{
			return std::distance(mods.begin(), it);
		}
		return -1;
	}

	IModManager::IModManager()
		:ManagerWithTypeInfo(GameModsModule::GetInstance())
	{
	}
}
