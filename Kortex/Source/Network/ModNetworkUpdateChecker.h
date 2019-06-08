#pragma once
#include "stdafx.h"
#include "Common.h"
#include "IModNetwork.h"
#include "ModSourceStore.h"
#include "NetworkModUpdateInfo.h"
#include <KxFramework/KxComponentSystem.h>

namespace Kortex
{
	class ModNetworkUpdateChecker: public KxComponentOf<IModNetwork>
	{
		public:
			using OnUpdateEvent = std::function<void(IGameMod& gameMod, NetworkModUpdateInfo& updateInfo)>;
			using OnUpdateDoneEvent = std::function<void()>;

		protected:
			const ModSourceItem* GetModSourceItemFromMod(const IGameMod& gameMod) const;

		public:
			virtual bool RunUpdateCheck(OnUpdateEvent onUpdate = {}, OnUpdateDoneEvent onDone = {}) = 0;

			virtual bool IsAutomaticCheckAllowed() const = 0;
			virtual wxDateTime GetLastAutomaticCheckDate() const = 0;
			virtual wxTimeSpan GetAutomaticCheckInterval() const = 0;
			bool CanIssueNewAutomaticCheck() const;

			virtual NetworkModUpdateInfo GetUpdateInfo(const NetworkModInfo& modInfo) const = 0;
			NetworkModUpdateInfo GetUpdateInfo(const IGameMod& gameMod) const;
	};
}
