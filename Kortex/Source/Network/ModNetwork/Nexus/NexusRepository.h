#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/ModNetworkRepository.h"
#include "NexusModInfo.h"
class KxCURLEvent;

namespace Kortex::NetworkManager
{
	class NexusModNetwork;
	class NexusUtility;
	class NexusAuth;

	class NexusRepository: public ModNetworkRepository
	{
		friend class NexusModNetwork;

		public:
			enum class UpdatedModsInterval
			{
				Default = 0,
				Day,
				Week,
				Month
			};

		private:
			NexusModNetwork& m_Nexus;
			NexusUtility& m_Utility;
			NexusAuth& m_Auth;

			ModRepositoryLimitsData m_LimitsData;

			wxTimer m_ModsUpdateCheckTimer;
			wxString m_LastUpdatedModsJson;

		protected:
			wxString ConvertEndorsementState(const ModEndorsement& state) const;
			void OnResponseHeader(KxCURLEvent& event);

			wxString GetLastUpdatedModsCacheFile() const;
			void OnModsUpdateCheck(UpdatedModsInterval interval = UpdatedModsInterval::Default);
			void DoInitialUpdateCheck(UpdatedModsInterval interval = UpdatedModsInterval::Default);

			void OnInit() override;
			void OnUninit() override;

		public:
			NexusRepository(NexusModNetwork& nexus, NexusUtility& utility, NexusAuth& auth)
				:m_Nexus(nexus), m_Utility(utility), m_Auth(auth)
			{
			}

		public:
			ModRepositoryLimits GetRequestLimits() const override
			{
				return m_LimitsData;
			}
			bool IsAutomaticUpdateCheckAllowed() const override;
			
			bool RestoreBrokenDownload(const KxFileItem& fileItem, IDownloadEntry& download) override;

			std::optional<ModInfoReply> GetModInfo(const ModRepositoryRequest& request) const override;
			std::optional<ModEndorsementReply> EndorseMod(const ModRepositoryRequest& request, ModEndorsement state) override;

			std::optional<ModFileReply> GetModFileInfo(const ModRepositoryRequest& request) const override;
			std::vector<ModFileReply> GetModFiles(const ModRepositoryRequest& request) const override;
			std::vector<ModDownloadReply> GetFileDownloads(const ModRepositoryRequest& request) const override;
	};
}
