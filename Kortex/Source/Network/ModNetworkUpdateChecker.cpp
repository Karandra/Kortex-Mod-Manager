#include "stdafx.h"
#include "ModNetworkUpdateChecker.h"
#include "GameMods/IGameMod.h"
#include "Utility/DateTime.h"

namespace Kortex
{
	const ModSourceItem* ModNetworkUpdateChecker::GetModSourceItemFromMod(const IGameMod& gameMod) const
	{
		return gameMod.GetModSourceStore().GetItem(GetContainer());
	}
	
	bool ModNetworkUpdateChecker::CanIssueNewAutomaticCheck() const
	{
		using namespace Utility;
		return DateTime::IsLaterThanBy(DateTime::Now(), GetLastAutomaticCheckDate(), GetAutomaticCheckInterval());
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
