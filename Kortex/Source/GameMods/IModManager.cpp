#include "stdafx.h"
#include "IModManager.h"
#include "GameMods/GameModsModule.h"
#include "Utility/KUPtrVectorUtil.h"

namespace Kortex
{
	namespace ModManager::Internal
	{
		const SimpleManagerInfo TypeInfo("ModManager", "ModManager.Name");
	}

	void IModManager::RecalcModIndexes(size_t startAt)
	{
		intptr_t priority = 0;
		for (auto& mod: GetMods())
		{
			mod->SetPriority(priority);
			priority++;
		}
	}
	IModManager::IModManager()
		:ManagerWithTypeInfo(GameModsModule::GetInstance())
	{
	}

	bool IModManager::MoveModsTo(const IGameMod::RefVector& toMove, const IGameMod& anchor)
	{
		if (KUPtrVectorUtil::MoveAfter(GetMods(), toMove, anchor))
		{
			RecalcModIndexes();
			return true;
		}
		return false;
	}
}
