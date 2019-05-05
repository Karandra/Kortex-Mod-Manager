#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/ModNetworkRepository.h"
#include "NexusNetworkReply.h"
#include <KxFramework/KxJSON.h>
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
			enum class ModActivity
			{
				Default = 0,
				Day,
				Week,
				Month
			};
			using GetModFiles2Result = std::pair<std::unordered_map<ModFileID, ModFileReply>, std::unordered_map<ModFileID, NexusModFileUpdateReply>>;

		private:
			NexusModNetwork& m_Nexus;
			NexusUtility& m_Utility;
			NexusAuth& m_Auth;

			ModRepositoryLimitsData m_LimitsData;

		protected:
			wxString ConvertEndorsementState(const ModEndorsement& state) const;
			void OnResponseHeader(KxCURLEvent& event);

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
			bool IsAutomaticUpdateCheckAllowed() const;
			bool RestoreBrokenDownload(const KxFileItem& fileItem, IDownloadEntry& download) override;

			std::optional<ModInfoReply> GetModInfo(const ModRepositoryRequest& request) const override;
			std::optional<ModEndorsementReply> EndorseMod(const ModRepositoryRequest& request, ModEndorsement state) override;

			std::optional<ModFileReply> GetModFileInfo(const ModRepositoryRequest& request) const override;
			std::vector<ModFileReply> GetModFiles(const ModRepositoryRequest& request) const override;
			std::vector<ModDownloadReply> GetFileDownloads(const ModRepositoryRequest& request) const override;

		public:
			std::optional<GetModFiles2Result> GetModFiles2(const ModRepositoryRequest& request, bool files, bool updates) const;
	};
}
