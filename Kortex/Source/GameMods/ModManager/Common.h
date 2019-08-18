#pragma once
#include "stdafx.h"
#include "Utility/EnumClassOperations.h"

namespace Kortex::ModManager
{
	enum class GetModsFlags: uint32_t
	{
		None = 0,
		ActiveOnly = 1 << 0,
		BaseGame = 1 << 1,
		MandatoryMods = 1 << 2,
		WriteTarget = 1 << 3,

		Everything = BaseGame|MandatoryMods|WriteTarget
	};
}

namespace Kortex
{
	Kortex_ImplementEnum(ModManager::GetModsFlags);
}
