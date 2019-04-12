#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/IModSource.h"
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
	class NexusProvider:
		public KxRTTI::IExtendInterface<NexusProvider, IModSource, IAuthenticableModSource>,
		public KxSingletonPtr<NexusProvider>
	{
		private:
			KxSecretDefaultStoreService m_CredentialsStore;
			std::unique_ptr<KxWebSocket::IClient> m_WebSocketClient;

			wxString m_UserToken;
			wxString m_UserAgent;
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
			KxCURLSession& ConfigureRequest(KxCURLSession& request, const wxString& apiKey = wxEmptyString) const;
			bool ShouldTryLater(const KxCURLReplyBase& reply) const;
			wxString GetAPIURL() const;
			wxString GetAPIKey(wxString* userName = nullptr) const;
			void RequestUserAvatar(Nexus::ValidationInfo& info);

		public:
			NexusProvider();

		public:
			// IModSource
			KImageEnum GetIcon() const override;
			wxString GetName() const override;
			wxString GetGameID(const GameID& id = GameIDs::NullGameID) const override;
			wxString GetGameID(const ProviderRequest& request) const
			{
				return GetGameID(request.GetGameID());
			}
			wxString& ConvertDescriptionToHTML(wxString& description) const override;
			wxString GetModURLBasePart(const GameID& id = GameIDs::NullGameID) const override;
			wxString GetModURL(const ProviderRequest& request) override;

		public:
			// IAuthenticableModSource
			bool IsAuthenticated() const override;
			bool Authenticate() override;
			bool ValidateAuth() override;
			bool SignOut() override;

		public:
			// IModSource
			std::unique_ptr<IModInfo> NewModInfo() const override
			{
				return std::make_unique<Nexus::ModInfo>();
			}
			std::unique_ptr<IModFileInfo> NewModFileInfo() const override
			{
				return std::make_unique<Nexus::ModFileInfo>();
			}
			std::unique_ptr<IModDownloadInfo> NewModDownloadInfo() const override
			{
				return std::make_unique<Nexus::ModDownloadInfo>();
			}
			std::unique_ptr<IModEndorsementInfo> NewModEndorsementInfo() const override
			{
				return std::make_unique<Nexus::ModEndorsementInfo>();
			}

			bool RestoreBrokenDownload(const wxString& filePath, IDownloadEntry& download);

			std::unique_ptr<IModInfo> GetModInfo(const ProviderRequest& request) const override;
			std::unique_ptr<IModFileInfo> GetFileInfo(const ProviderRequest& request) const override;
			IModFileInfo::Vector GetFilesList(const ProviderRequest& request) const override;
			IModDownloadInfo::Vector GetFileDownloadLinks(const ProviderRequest& request) const override;
			std::unique_ptr<IModEndorsementInfo> EndorseMod(const ProviderRequest& request, ModEndorsement state) override;

			std::unique_ptr<Nexus::ValidationInfo> GetValidationInfo(const wxString& apiKey = wxEmptyString) const;
			std::unique_ptr<Nexus::GameInfo> GetGameInfo(const GameID& id = GameIDs::NullGameID) const;
			Nexus::GameInfo::Vector GetGamesList() const;
			Nexus::IssueInfo::Vector GetIssues() const;

			GameID TranslateNxmGameID(const wxString& id) const;
			wxString ConstructNXM(const IModFileInfo& fileInfo, const GameID& id = GameIDs::NullGameID) const;
	};
}
