#include "stdafx.h"
#include "ModNetworkUpdateChecker.h"
#include "GameMods/IGameMod.h"

namespace Kortex
{
	const ModSourceItem* ModNetworkUpdateChecker::GetModSourceItemFromMod(const IGameMod& gameMod) const
	{
		return gameMod.GetModSourceStore().GetItem(GetContainer());
	}

	bool ModNetworkUpdateChecker::HasNewVesion(const IGameMod& gameMod) const
	{
		if (const ModSourceItem* item = GetModSourceItemFromMod(gameMod))
		{
			return HasNewVesion(item->GetModInfo());
		}
		return false;
	}
	wxDateTime ModNetworkUpdateChecker::GetLastUpdateCheck(const IGameMod& gameMod) const
	{
		if (const ModSourceItem* item = GetModSourceItemFromMod(gameMod))
		{
			return GetLastUpdateCheck(item->GetModInfo());
		}
		return {};
	}
}
