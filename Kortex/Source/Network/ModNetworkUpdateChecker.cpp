#include "stdafx.h"
#include "ModNetworkUpdateChecker.h"
#include "GameMods/IGameMod.h"

namespace Kortex
{
	const ModSourceItem* ModNetworkUpdateChecker::GetModSourceItemFromMod(const IGameMod& gameMod) const
	{
		return gameMod.GetModSourceStore().GetItem(GetContainer());
	}

	NetworkModUpdateInfo ModNetworkUpdateChecker::GetUpdateInfo(const IGameMod& gameMod) const
	{
		if (const ModSourceItem* item = GetModSourceItemFromMod(gameMod))
		{
			return GetUpdateInfo(item->GetModInfo());
		}
		return {};
	}
}
