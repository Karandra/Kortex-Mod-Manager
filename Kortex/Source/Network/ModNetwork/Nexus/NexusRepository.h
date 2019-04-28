#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/ModNetworkRepository.h"
#include "NexusModInfo.h"

namespace Kortex::NetworkManager
{
	class NexusModNetwork;
	class NexusUtility;

	class NexusRepository: public ModNetworkRepository
	{
		friend class NexusModNetwork;

		private:
			NexusModNetwork& m_Nexus;
			NexusUtility& m_Utility;
			ModRepositoryLimitsData m_LimitsData;

		private:
			wxString ConvertEndorsementState(const ModEndorsement& state) const;

		public:
			NexusRepository(NexusModNetwork& nexus, NexusUtility& utility)
				:m_Nexus(nexus), m_Utility(utility)
			{
			}

		public:
			ModRepositoryLimits GetRequestLimits() const override
			{
				return m_LimitsData;
			}
			bool RestoreBrokenDownload(const KxFileItem& fileItem, IDownloadEntry& download) override;

			std::optional<ModInfoReply> GetModInfo(const ModRepositoryRequest& request) const override;
			std::optional<ModEndorsementReply> EndorseMod(const ModRepositoryRequest& request, ModEndorsement state) override;

			std::optional<ModFileReply> GetModFileInfo(const ModRepositoryRequest& request) const override;
			std::vector<ModFileReply> GetModFiles(const ModRepositoryRequest& request) const override;
			std::vector<ModDownloadReply> GetFileDownloads(const ModRepositoryRequest& request) const override;
	};
}
