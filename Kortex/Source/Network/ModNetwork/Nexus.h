#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/IModNetwork.h"
#include "Network/IModNetworkRepository.h"
#include "Network/IAuthenticableModNetwork.h"
#include "NexusModInfo.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxUUID.h>
class KxCURLEvent;
class KxCURLSession;
class KxCURLReplyBase;

namespace KxWebSocket
{
	class IClient;
}

namespace Kortex::NetworkManager
{
	class NexusModNetwork:
		public KxRTTI::IExtendInterface<NexusModNetwork, IModNetwork, IAuthenticableModNetwork, IModNetworkRepository>,
		public KxSingletonPtr<NexusModNetwork>
	{
		private:
			KxSecretDefaultStoreService m_CredentialsStore;
			std::unique_ptr<KxWebSocket::IClient> m_WebSocketClient;

			wxString m_UserToken;
			KxUUID m_SessionGUID;
			bool m_IsAuthenticated = false;

			ModRepositoryLimitsData m_LimitsData;

		protected:
			// IAuthenticableModSource
			wxWindow* GetInvokingWindow() const override;
			KxStandardID OnAuthSuccess() override;
			KxStandardID OnAuthFail() override;
			KxSecretDefaultStoreService& GetSecretStore() override
			{
				return m_CredentialsStore;
			}
			
		private:
			wxString EndorsementStateToString(const ModEndorsement& state) const;
			wxString GetAPIURL() const;
			wxString GetAPIKey(wxString* userName = nullptr) const;

			void RequestUserAvatar(const NexusValidationReply& info);
			std::unique_ptr<KxCURLSession> NewCURLSession(const wxString& address, const wxString& apiKey = {}) const;
			void OnResponseHeader(KxCURLEvent& event);

		public:
			NexusModNetwork();

		public:
			// IModNetwork
			KImageEnum GetIcon() const override;
			wxString GetName() const override;

			wxString TranslateGameIDToNetwork(const GameID& id = {}) const override;
			wxString TranslateGameIDToNetwork(const ModRepositoryRequest& request) const
			{
				return TranslateGameIDToNetwork(request.GetGameID());
			}
			GameID TranslateGameIDFromNetwork(const wxString& id) const override;
			void ConvertDescriptionText(wxString& description) const override;

			wxString GetModPageBaseURL(const GameID& id = {}) const override;
			wxString GetModPageURL(const ModRepositoryRequest& request) override;

		public:
			// IAuthenticableModSource
			bool IsAuthenticated() const override;
			bool Authenticate() override;
			bool ValidateAuth() override;
			bool SignOut() override;

		public:
			// IModNetworkRepository
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

		public:
			// NexusModNetwork
			std::optional<NexusValidationReply> GetValidationInfo(const wxString& apiKey = {}) const;
			std::optional<NexusGameReply> GetGameInfo(const GameID& id = {}) const;
			std::vector<NexusGameReply> GetGamesList() const;

			wxString ConstructNXM(const NetworkModInfo& modInfo, const GameID& id = {}, const NexusNXMLinkData& linkData = {}) const;
			bool ParseNXM(const wxString& link, GameID& gameID, NetworkModInfo& modInfo, NexusNXMLinkData& linkData);
	};
}
