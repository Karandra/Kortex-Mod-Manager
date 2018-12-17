#pragma once
#include "stdafx.h"
#include "FixedGameMod.h"

namespace Kortex::ModManager
{
	class KMandatoryModEntry: public FixedGameMod
	{
		public:
			KMandatoryModEntry(intptr_t priority = -1)
				:FixedGameMod(priority)
			{
			}

		public:
			virtual bool IsLinkedMod() const override
			{
				return true;
			}
			virtual wxString GetModFilesDir() const override
			{
				return KVarExp(FixedGameMod::GetModFilesDir());
			}
	};
}
