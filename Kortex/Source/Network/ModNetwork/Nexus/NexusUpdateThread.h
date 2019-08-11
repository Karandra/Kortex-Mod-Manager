#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/NetworkModInfo.h"
#include "Network/NetworkModUpdateInfo.h"
#include "NexusRepository.h"
#include "NexusNetworkReply.h"
#include <Kx/Threading/Thread.h>

namespace Kortex::NetworkManager
{
	class NexusUpdateChecker;

	class NexusUpdateThread final
	{
		public:
			enum class ModActivity
			{
				Default = 0,
				Day,
				Week,
				Month
			};
			using ActivityMap = std::unordered_map<ModID, NexusModActivityReply>;
			using UpdateInfoMap = std::unordered_map<NetworkModInfo, NetworkModUpdateInfo>;

		private:
			NexusRepository& m_Repository;
			NexusUpdateChecker& m_UpdateChecker;
			KxThread m_Thread;

			wxDateTime m_MonthlyModActivityDate;
			std::optional<ActivityMap> m_MonthlyModActivity;

			std::unordered_map<ModID::TValue, ModInfoReply> m_InfoReplies;
			std::unordered_map<ModID::TValue, NexusRepository::GetModFiles2Result> m_FileReplies;

			UpdateInfoMap m_UpdateInfo;
			wxDateTime m_CurrentDate;
			size_t m_UpdatesCount = 0;

		private:
			const ModInfoReply* GetOrQueryModInfo(ModID id);;
			const NexusRepository::GetModFiles2Result* GetOrQueryModFiles(ModID id);
			void OnUpdateChecked(NetworkModUpdateInfo& updateInfo, ModUpdateState state, std::optional<KxVersion> version = {});;
			void CheckForSingleUpdate(NetworkModUpdateInfo& updateInfo, IGameMod& gameMod, const NetworkModInfo& modInfo);

			void OnExecute(KxThreadEvent& event);

		public:
			NexusUpdateThread(NexusRepository& repository, NexusUpdateChecker& updateChecker);

		public:
			KxRefEvtHandler GetEvtHandler()
			{
				return &m_Thread.GetThisHandler();
			}
			bool Run(const UpdateInfoMap& updateInfo);

			wxDateTime GetCheckDate() const
			{
				return m_CurrentDate;
			}
			size_t GetUpdatesCount() const
			{
				return m_UpdatesCount;
			}
			UpdateInfoMap&& TakeUpdateInfo()
			{
				return std::move(m_UpdateInfo);
			}
	};
}
