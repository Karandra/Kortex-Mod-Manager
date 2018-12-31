#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/INetworkProvider.h"
#include "NexusModInfo.h"
#include <KxFramework/KxSingleton.h>
class KxCURLSession;
class KxCURLReplyBase;

//////////////////////////////////////////////////////////////////////////
namespace Kortex::NetworkManager
{
	class NexusProvider: public INetworkProvider, public KxSingletonPtr<NexusProvider>
	{
		public:
			static constexpr NetworkProviderID GetTypeID()
			{
				return NetworkProviderIDs::Nexus;
			}

		private:
			void OnAuthSuccess(wxWindow* window = nullptr);
			void OnAuthFail(wxWindow* window = nullptr);
		
			wxString EndorsementStateToString(const ModEndorsement& state) const;
			KxCURLSession& ConfigureRequest(KxCURLSession& request, const wxString& apiKey = wxEmptyString) const;
			bool ShouldTryLater(const KxCURLReplyBase& reply) const;
			wxString GetAPIURL() const;
			wxString GetAPIKey(wxString* userName = nullptr) const;
			void RequestUserAvatar(Nexus::ValidationInfo& info);

		protected:
			virtual bool DoAuthenticate(wxWindow* window = nullptr) override;
			virtual bool DoValidateAuth(wxWindow* window = nullptr) override;
			virtual bool DoSignOut(wxWindow* window = nullptr) override;
			virtual bool DoIsAuthenticated() const override;

		public:
			NexusProvider();

		public:
			KImageEnum GetIcon() const override;
			wxString GetName() const override;
			wxString GetGameID(const GameID& id = GameIDs::NullGameID) const override;
			wxString& ConvertDescriptionToHTML(wxString& description) const override;
			wxString GetModURLBasePart(const GameID& id = GameIDs::NullGameID) const override;
			wxString GetModURL(ModID modID, const wxString& modSignature = wxEmptyString, const GameID& id = GameIDs::NullGameID) override;

		public:
			std::unique_ptr<IModInfo> NewModInfo() const
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

			std::unique_ptr<IModInfo> GetModInfo(ModID modID, const wxAny& extraInfo = wxAny(), const GameID& id = GameIDs::NullGameID) const override;
			std::unique_ptr<IModFileInfo> GetFileInfo(ModID modID, ModFileID fileID, const wxAny& extraInfo = wxAny(), const GameID& id = GameIDs::NullGameID) const override;
			IModFileInfo::Vector GetFilesList(ModID modID, const wxAny& extraInfo = wxAny(), const GameID& id = GameIDs::NullGameID) const override;
			IModDownloadInfo::Vector GetFileDownloadLinks(ModID modID, ModFileID fileID, const wxAny& extraInfo = wxAny(), const GameID& id = GameIDs::NullGameID) const override;
			std::unique_ptr<IModEndorsementInfo> EndorseMod(ModID modID, ModEndorsement state, const wxAny& extraInfo = wxAny(), const GameID& id = GameIDs::NullGameID) override;

			std::unique_ptr<Nexus::ValidationInfo> GetValidationInfo(const wxString& apiKey = wxEmptyString) const;
			std::unique_ptr<Nexus::GameInfo> GetGameInfo(const GameID& id = GameIDs::NullGameID) const;
			Nexus::GameInfo::Vector GetGamesList() const;
			Nexus::IssueInfo::Vector GetIssues() const;

			GameID TranslateNxmGameID(const wxString& id) const;
			wxString ConstructNXM(const IModFileInfo& fileInfo, const GameID& id = GameIDs::NullGameID) const;
	};
}
