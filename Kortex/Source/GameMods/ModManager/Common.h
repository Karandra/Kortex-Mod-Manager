#pragma once
#include "stdafx.h"
#include "Utility/EnumClassOperations.h"

namespace Kortex::ModManager
{
	enum class GetModsFlags
	{
		None = 0,
		ActiveOnly = 1 << 0,
		WriteTarget = 1 << 1,
		BaseGame = 1 << 2,
		MandatoryMods = 1 << 3,

		Everything = ActiveOnly|WriteTarget|BaseGame|MandatoryMods
	};
}

namespace Kortex
{
	Kortex_ImplementEnum(ModManager::GetModsFlags);
}
