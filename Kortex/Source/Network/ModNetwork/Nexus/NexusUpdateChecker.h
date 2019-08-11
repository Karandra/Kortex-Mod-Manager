#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/ModNetworkUpdateChecker.h"
#include "Network/NetworkModInfo.h"
#include "NexusNetworkReply.h"
#include "NexusUpdateThread.h"
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
		friend class NexusUpdateThread;

		public:
			using ModActivity = NexusUpdateThread::ModActivity;
			using ActivityMap = NexusUpdateThread::ActivityMap;

		private:
			NexusModNetwork& m_Nexus;
			NexusUtility& m_Utility;
			NexusRepository& m_Repository;

			KxTimerMethod<NexusUpdateChecker> m_Timer;
			wxTimeSpan m_TimeElapsed;
			wxTimeSpan m_AutomaticCheckInterval;
			std::unordered_map<NetworkModInfo, NetworkModUpdateInfo> m_UpdateInfo;

			NexusUpdateThread m_Thread;
			wxDateTime m_LastCheckDate;
			bool m_UpdateCheckInProgress = false;

		private:
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& networkNode);
			std::optional<ActivityMap> GetModsActivityFor(ModActivity interval) const;

		private:
			void OnInit() override;
			void OnUninit() override;
			void OnTimer(wxTimerEvent& event);
			void OnThreadFinished(KxThreadEvent& event);

			wxString GetUpdateInfoFile() const;
			bool SaveUpdateInfo();
			bool LoadUpdateInfo();

			NetworkModUpdateInfo* GetUpdateInfoPtr(const NetworkModInfo& modInfo);
			const NetworkModUpdateInfo* GetUpdateInfoPtr(const NetworkModInfo& modInfo) const;

		public:
			NexusUpdateChecker(NexusModNetwork& nexus, NexusUtility& utility, NexusRepository& repository);
			
		public:
			bool RunUpdateCheck() override;
			
			bool IsAutomaticCheckAllowed() const override;
			wxTimeSpan GetAutomaticCheckInterval() const override;
			wxDateTime GetLastAutomaticCheckDate() const override
			{
				return m_LastCheckDate;
			}

			NetworkModUpdateInfo GetUpdateInfo(const NetworkModInfo& modInfo) const override;
	};
}
