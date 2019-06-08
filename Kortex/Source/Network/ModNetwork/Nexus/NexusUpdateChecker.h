#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/ModNetworkUpdateChecker.h"
#include "Network/NetworkModInfo.h"
#include "NexusNetworkReply.h"
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
			using ActivityMap = std::unordered_map<ModID, NexusModActivityReply>;

		private:
			NexusModNetwork& m_Nexus;
			NexusUtility& m_Utility;
			NexusRepository& m_Repository;

			KxTimerMethod<NexusUpdateChecker> m_Timer;
			wxTimeSpan m_TimeElapsed;
			wxTimeSpan m_AutomaticCheckInterval;
			std::unordered_map<NetworkModInfo, NetworkModUpdateInfo> m_UpdateInfo;

			std::optional<ActivityMap> m_MonthlyModActivity;
			wxDateTime m_MonthlyModActivityDate;
			bool m_UpdateCheckInProgress = false;

		private:
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& networkNode);

			wxString ModActivityToString(ModActivity interval) const;
			std::optional<ActivityMap> GetModsActivityFor(ModActivity interval) const;

			void DoRunUpdateCheckEntry(OnUpdateEvent onUpdate, size_t& updatesCount);
			bool DoRunUpdateCheck(OnUpdateEvent onUpdate = {}, OnUpdateDoneEvent onDone = {});

		private:
			void OnInit() override;
			void OnUninit() override;
			void OnTimer(wxTimerEvent& event);

			wxString GetUpdateInfoFile() const;
			bool SaveUpdateInfo();
			bool LoadUpdateInfo();

			NetworkModUpdateInfo* GetUpdateInfoPtr(const NetworkModInfo& modInfo);
			const NetworkModUpdateInfo* GetUpdateInfoPtr(const NetworkModInfo& modInfo) const;

		public:
			NexusUpdateChecker(NexusModNetwork& nexus, NexusUtility& utility, NexusRepository& repository)
				:m_Nexus(nexus), m_Utility(utility), m_Repository(repository)
			{
			}
			
		public:
			bool RunUpdateCheck(OnUpdateEvent onUpdate = {}, OnUpdateDoneEvent onDone = {}) override;
			
			bool IsAutomaticCheckAllowed() const override;
			wxTimeSpan GetAutomaticCheckInterval() const override;

			NetworkModUpdateInfo GetUpdateInfo(const NetworkModInfo& modInfo) const override;
	};
}
