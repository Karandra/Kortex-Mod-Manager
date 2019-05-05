#pragma once
#include "stdafx.h"
#include "Common.h"
#include "IModNetwork.h"
#include "ModSourceStore.h"
#include <KxFramework/KxComponentSystem.h>

namespace Kortex
{
	class ModNetworkUpdateChecker: public KxComponentOf<IModNetwork>
	{
		protected:
			const ModSourceItem* GetModSourceItemFromMod(const IGameMod& gameMod) const;

		public:
			virtual bool IsAutomaticCheckAllowed() const = 0;
			virtual int64_t GetAutomaticCheckInterval() const = 0;

			virtual bool HasNewVesion(const NetworkModInfo& modInfo) const = 0;
			bool HasNewVesion(const IGameMod& gameMod) const;

			virtual wxDateTime GetLastUpdateCheck(const NetworkModInfo& modInfo) const = 0;
			wxDateTime GetLastUpdateCheck(const IGameMod& gameMod) const;
	};
}
