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

			mutable wxCriticalSection m_LimitsDataCS;
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
			ModRepositoryLimits GetRequestLimits() const override;
			bool IsAutomaticUpdateCheckAllowed() const;
			bool ParseDownloadName(const wxString& name, ModFileReply& result);

			bool QueryDownload(const KxFileItem& fileItem, const DownloadItem& download, ModFileReply& fileReply) override;
			void OnToolBarMenu(KxMenu& menu);
			void OnDownloadMenu(KxMenu& menu, DownloadItem* download = nullptr) override;

			bool QueueDownload(const wxString& link) override;
			wxAny GetDownloadTarget(const wxString& link) override;

			std::optional<ModInfoReply> GetModInfo(const ModRepositoryRequest& request) const override;
			std::optional<ModEndorsementReply> EndorseMod(const ModRepositoryRequest& request, ModEndorsement state) override;

			std::optional<ModFileReply> GetModFileInfo(const ModRepositoryRequest& request) const override;
			std::vector<ModFileReply> GetModFiles(const ModRepositoryRequest& request) const override;
			std::optional<GetModFiles2Result> GetModFiles2(const ModRepositoryRequest& request, bool files, bool updates) const;
			std::vector<ModDownloadReply> GetFileDownloads(const ModRepositoryRequest& request) const override;

		public:
			KxURI ConstructNXM(const NetworkModInfo& modInfo, const GameID& id = {}, const NexusNXMLinkData& linkData = {}) const;
			bool ParseNXM(const wxString& link, GameID& gameID, NetworkModInfo& modInfo, NexusNXMLinkData& linkData) const;
			bool ParseNXM(const KxURI& uri, GameID& gameID, NetworkModInfo& modInfo, NexusNXMLinkData& linkData) const
			{
				return ParseNXM(uri.BuildUnescapedURI(), gameID, modInfo, linkData);
			}
			
			void ConfigureNXMHandler();
	};
}
