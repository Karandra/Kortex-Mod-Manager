#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/ModNetworkUpdateChecker.h"
#include "Network/NetworkModInfo.h"
#include <KxFramework/KxJSON.h>
#include <KxFramework/KxTimer.h>

namespace Kortex
{
	class IGameMod;
	class ModSourceItem;
}

namespace Kortex::NetworkManager
{
	class NexusModNetwork;
	class NexusUtility;
	class NexusRepository;

	class NexusUpdateChecker: public ModNetworkUpdateChecker
	{
		friend class NexusModNetwork;

		public:
			enum class ModActivity
			{
				Default = 0,
				Day,
				Week,
				Month
			};
			
		private:
			NexusModNetwork& m_Nexus;
			NexusUtility& m_Utility;
			NexusRepository& m_Repository;

			KxTimerMethod<NexusUpdateChecker> m_Timer;
			wxTimeSpan m_TimeElapsed;
			wxTimeSpan m_AutomaticCheckInterval;
			KxJSONObject m_MonthlyModActivity;
			std::unordered_map<NetworkModInfo, std::pair<wxDateTime, bool>> m_UpdateInfo;

		private:
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& networkNode);

			wxString ModActivityToString(ModActivity interval) const;
			wxString GetModsActivityFor(ModActivity interval);
			const KxJSONObject* GetOrQueryActivity(ModActivity interval);

			void CheckModUpdates();

		private:
			void OnInit() override;
			void OnUninit() override;
			void OnTimer();

		public:
			NexusUpdateChecker(NexusModNetwork& nexus, NexusUtility& utility, NexusRepository& repository)
				:m_Nexus(nexus), m_Utility(utility), m_Repository(repository)
			{
			}
	
		public:
			bool IsAutomaticCheckAllowed() const override;
			wxTimeSpan GetAutomaticCheckInterval() const override;

			bool HasNewVesion(const NetworkModInfo& modInfo) const override;

			wxDateTime GetLastUpdateCheck(const NetworkModInfo& modInfo) const override;
			void SetLastUpdateCheck(const ModSourceItem& sourceItem, const wxDateTime& date, bool hasNewVersion);
	};
}
