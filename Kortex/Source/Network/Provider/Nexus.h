#pragma once
#include "stdafx.h"
#include "Network/NetworkConstants.h"
#include "Network/INetworkProvider.h"
#include "NexusModInfo.h"
#include <KxFramework/KxSingleton.h>
class KxCURLSession;
class KxCURLReplyBase;

//////////////////////////////////////////////////////////////////////////
namespace Kortex::Network
{
	class NexusProvider: public INetworkProvider, public KxSingletonPtr<NexusProvider>
	{
		public:
			static constexpr ProviderID GetTypeID()
			{
				return ProviderIDs::Nexus;
			}

		private:
			void OnAuthSuccess(wxWindow* window = nullptr);
			void OnAuthFail(wxWindow* window = nullptr);

			template<class T> wxDateTime ReadDateTime(const T& json) const
			{
				wxDateTime date;
				date.ParseISOCombined(json.get<wxString>());
				return date.FromUTC(date.IsDST());
			}
			template<class T> void ReadGameInfo(const T& json, GameInfo& info) const
			{
				info.m_ID = json["id"];
				info.m_Name = json["name"].get<wxString>();
				info.m_Genre = json["genre"].get<wxString>();
				info.m_ForumURL = json["forum_url"].get<wxString>();
				info.m_NexusURL = json["nexusmods_url"].get<wxString>();
				info.m_DomainName = json["domain_name"].get<wxString>();

				info.m_FilesCount = json["file_count"];
				info.m_DownloadsCount = json["downloads"];
				info.m_ModsCount = json["mods"];
				info.m_ApprovedDate = wxDateTime((time_t)json["approved_date"]);
			}
			template<class T> void ReadFileInfo(const T& json, FileInfo& info) const
			{
				info.m_ID = json["file_id"];
				info.m_IsPrimary = json["is_primary"];
				info.m_Name = json["file_name"].get<wxString>();
				info.m_DisplayName = json["name"].get<wxString>();
				info.m_Version = json["version"].get<wxString>();
				info.m_ChangeLog = json["changelog_html"].get<wxString>();
				info.m_UploadDate = ReadDateTime(json["uploaded_time"]);

				ConvertDisplayName(info.m_DisplayName);
				ConvertChangeLog(info.m_ChangeLog);

				// WTF?! Why file size is in kilobytes instead of bytes?
				// Conversion not exact, final size may be a bit smaller.
				// At least download manager can request correct file size upon downloading
				info.m_Size = json["size"].get<int64_t>() * 1024;
			
				// Values: 'MAIN', 'OPTIONAL'.
				info.m_Category = json["category_name"].get<wxString>();
			}
		
			wxString EndorsementStateToString(EndorsementState::Value state) const;
			KxCURLSession& ConfigureRequest(KxCURLSession& request, const wxString& apiKey = wxEmptyString) const;
			bool ShouldTryLater(const KxCURLReplyBase& reply) const;
			wxString GetAPIURL() const;
			wxString GetAPIKey(wxString* userName = nullptr) const;
			void RequestUserAvatar(ValidationInfo& info);

			wxString& ConvertChangeLog(wxString& changeLog) const;
			wxString& ConvertDisplayName(wxString& name) const;

		protected:
			virtual bool DoAuthenticate(wxWindow* window = nullptr) override;
			virtual bool DoValidateAuth(wxWindow* window = nullptr) override;
			virtual bool DoSignOut(wxWindow* window = nullptr) override;
			virtual bool DoIsAuthenticated() const override;

		public:
			NexusProvider();

		public:
			virtual KImageEnum GetIcon() const override;
			virtual wxString GetName() const override;
			virtual wxString GetGameID(const GameID& id = GameIDs::NullGameID) const override;
			virtual wxString& ConvertDescriptionToHTML(wxString& description) const override;
			virtual wxString GetModURLBasePart(const GameID& id = GameIDs::NullGameID) const override;
			virtual wxString GetModURL(int64_t modID, const wxString& modSignature = wxEmptyString, const GameID& id = GameIDs::NullGameID) override;

			virtual ModInfo GetModInfo(int64_t modID, const GameID& id = GameIDs::NullGameID) const override;
			virtual FileInfo GetFileItem(int64_t modID, int64_t fileID, const GameID& id = GameIDs::NullGameID) const override;
			virtual FileInfo::Vector GetFilesList(int64_t modID, const GameID& id = GameIDs::NullGameID) const override;
			virtual DownloadInfo::Vector GetFileDownloadLinks(int64_t modID, int64_t fileID, const GameID& id = GameIDs::NullGameID) const override;
			virtual EndorsementInfo EndorseMod(int64_t modID, EndorsementState::Value state = EndorsementState::Endorse, const GameID& id = GameIDs::NullGameID) override;

			ValidationInfo GetValidationInfo(const wxString& apiKey = wxEmptyString) const;
			GameInfo GetGameInfo(const GameID& id = GameIDs::NullGameID) const;
			GameInfo::Vector GetGamesList() const;
			IssueInfo::Vector GetIssues() const;

			wxString ConstructNXM(const FileInfo& fileInfo, const GameID& id = GameIDs::NullGameID) const;
	};
}
