#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/IModSource.h"
#include "Network/IModRepository.h"
#include "Network/IAuthenticableModSource.h"
#include "NexusModInfo.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxUUID.h>

class KxCURLSession;
class KxCURLReplyBase;

namespace KxWebSocket
{
	class IClient;
}

namespace Kortex::NetworkManager
{
	class NexusSource:
		public KxRTTI::IExtendInterface<NexusSource, IModSource, IAuthenticableModSource, IModRepository>,
		public KxSingletonPtr<NexusSource>
	{
		private:
			KxSecretDefaultStoreService m_CredentialsStore;
			std::unique_ptr<KxWebSocket::IClient> m_WebSocketClient;

			wxString m_UserToken;
			KxUUID m_SessionGUID;
			bool m_IsAuthenticated = false;

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

		public:
			NexusSource();

		public:
			// IModSource
			KImageEnum GetIcon() const override;
			wxString GetName() const override;
			wxString GetGameID(const GameID& id = {}) const override;
			wxString GetGameID(const ModRepositoryRequest& request) const
			{
				return GetGameID(request.GetGameID());
			}
			wxString& ConvertDescriptionToHTML(wxString& description) const override;
			wxString GetModURLBasePart(const GameID& id = {}) const override;
			wxString GetModURL(const ModRepositoryRequest& request) override;

		public:
			// IAuthenticableModSource
			bool IsAuthenticated() const override;
			bool Authenticate() override;
			bool ValidateAuth() override;
			bool SignOut() override;

		public:
			// IModRepository
			bool RestoreBrokenDownload(const KxFileItem& fileItem, IDownloadEntry& download) override;

			std::optional<ModInfoReply> GetModInfo(const ModRepositoryRequest& request) const override;
			std::optional<ModEndorsementReply> EndorseMod(const ModRepositoryRequest& request, ModEndorsement state) override;

			std::optional<ModFileReply> GetModFileInfo(const ModRepositoryRequest& request) const override;
			std::vector<ModFileReply> GetModFiles(const ModRepositoryRequest& request) const override;
			std::vector<ModDownloadReply> GetFileDownloads(const ModRepositoryRequest& request) const override;

		public:
			// NexusSource
			std::optional<NexusValidationReply> GetValidationInfo(const wxString& apiKey = wxEmptyString) const;
			std::optional<NexusGameReply> GetGameInfo(const GameID& id = {}) const;
			std::vector<NexusGameReply> GetGamesList() const;

			GameID TranslateNxmGameID(const wxString& id) const;
			wxString ConstructNXM(const NetworkModInfo& modInfo, const GameID& id = {}, const NexusNXMLinkData& linkData = {}) const;
			bool ParseNXM(const wxString& link, GameID& gameID, NetworkModInfo& modInfo, NexusNXMLinkData& linkData);
	};
}
