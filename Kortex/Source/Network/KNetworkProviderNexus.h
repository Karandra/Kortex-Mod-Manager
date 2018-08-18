#pragma once
#include "stdafx.h"
#include "KNetworkConstants.h"
#include "KNetworkProvider.h"
#include <KxFramework/KxSingleton.h>
class KNetworkProviderNexus;
class KxCURLSession;
class KxCURLReplyBase;

namespace KNetworkProviderNexusNS
{
	using InfoBase = KNetworkProviderNS::InfoBase;

	class ValidationInfo: public InfoBase
	{
		friend class KNetworkProviderNexus;

		private:
			wxString m_UserName;
			wxString m_APIKey;
			wxString m_EMail;
			wxString m_ProfilePictureURL;
			int64_t m_UserID = -1;
			bool m_IsPremium = false;
			bool m_IsSupporter = false;

		public:
			virtual bool IsOK() const override
			{
				return m_UserID >= 0;
			}
			virtual void Reset() override
			{
				*this = ValidationInfo();
			}

		public:
			int64_t GetUserID() const
			{
				return m_UserID;
			}
			bool IsPremium() const
			{
				return m_IsPremium;
			}
			bool IsSupporter() const
			{
				return m_IsSupporter;
			}
			
			const wxString& GetAPIKey() const
			{
				return m_APIKey;
			}
			const wxString& GetUserName() const
			{
				return m_UserName;
			}
			const wxString& GetEMail() const
			{
				return m_EMail;
			}
			const wxString& GetProfilePictureURL() const
			{
				return m_ProfilePictureURL;
			}
	};
	class GameInfo: public InfoBase
	{
		friend class KNetworkProviderNexus;
		
		public:
			using Vector = std::vector<GameInfo>;

		private:
			int64_t m_ID = -1;
			wxString m_Name;
			wxString m_Genre;
			wxString m_ForumURL;
			wxString m_NexusURL;
			wxString m_DomainName;

			int64_t m_FilesCount = -1;
			int64_t m_DownloadsCount = -1;
			int64_t m_ModsCount = -1;
			wxDateTime m_ApprovedDate;

		public:
			virtual bool IsOK() const override
			{
				return m_ID > 0 && m_FilesCount >= 0 && m_DownloadsCount >= 0 && m_ModsCount >= 0 && m_ApprovedDate.IsValid();
			}
			virtual void Reset() override
			{
				*this = GameInfo();
			}

		public:
			int GetID() const
			{
				return m_ID;
			}
			const wxString& GetName() const
			{
				return m_Name;
			}
			const wxString& GetGenre() const
			{
				return m_Genre;
			}
			const wxString& GetForumURL() const
			{
				return m_ForumURL;
			}
			const wxString& GetNexusURL() const
			{
				return m_NexusURL;
			}
			const wxString& GetDomainName() const
			{
				return m_DomainName;
			}

			int64_t GetFilesCount() const
			{
				return m_FilesCount;
			}
			int64_t GetDownloadsCount() const
			{
				return m_DownloadsCount;
			}
			int64_t GetModsCount() const
			{
				return m_ModsCount;
			}
			wxDateTime GetApprovedDate() const
			{
				return m_ApprovedDate;
			}
	};
	class IssueInfo: public InfoBase
	{
		friend class KNetworkProviderNexus;

		public:
			using Vector = std::vector<IssueInfo>;

		private:
			

		public:
			virtual bool IsOK() const override
			{
				return false;
			}
			virtual void Reset() override
			{
				*this = IssueInfo();
			}
	};
}

//////////////////////////////////////////////////////////////////////////
class KNetworkProviderNexus: public KNetworkProvider, public KxSingletonPtr<KNetworkProviderNexus>
{
	public:
		using ValidationInfo = KNetworkProviderNexusNS::ValidationInfo;
		using GameInfo = KNetworkProviderNexusNS::GameInfo;
		using IssueInfo = KNetworkProviderNexusNS::IssueInfo;

	public:
		static constexpr KNetworkProviderID GetTypeID()
		{
			return KNETWORK_PROVIDER_ID_NEXUS;
		}

	private:
		void OnAuthSuccess(wxWindow* window = NULL);
		void OnAuthFail(wxWindow* window = NULL);

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

			// WTF?! Why file size is in kilobytes instead of bytes?
			// Conversion not exact, final size may be a bit smaller.
			info.m_Size = json["size"].get<int64_t>() * 1024;
			
			// Values: 'MAIN', 'OPTIONAL'.
			//info.m_Category = json["category_name"].get<wxString>();
		}
		
		wxString EndorsementStateToString(EndorsementState::Value state) const;
		KxCURLSession& ConfigureRequest(KxCURLSession& request, const wxString& apiKey = wxEmptyString) const;
		bool ShouldTryLater(const KxCURLReplyBase& reply) const;
		wxString GetAPIURL() const;
		wxString GetAPIKey(wxString* userName = NULL) const;
		void RequestUserAvatar(ValidationInfo& info);

	protected:
		virtual bool DoAuthenticate(wxWindow* window = NULL) override;
		virtual bool DoValidateAuth(wxWindow* window = NULL) override;
		virtual bool DoSignOut(wxWindow* window = NULL) override;
		virtual bool DoIsAuthenticated() const override;

	public:
		KNetworkProviderNexus(KNetworkProviderID providerID);

	public:
		virtual KNetworkProviderID GetID() const override
		{
			return GetTypeID();
		}
		virtual KImageEnum GetIcon() const override;
		virtual wxString GetName() const override;
		virtual wxString GetGameID(const KProfileID& id = KProfileIDs::NullProfileID) const override;
		virtual wxString& ConvertDescriptionToHTML(wxString& description) const override;
		virtual wxString GetModURLBasePart(const KProfileID& id = KProfileIDs::NullProfileID) const override;
		virtual wxString GetModURL(int64_t modID, const wxString& modSignature = wxEmptyString, const KProfileID& id = KProfileIDs::NullProfileID) override;

		virtual ModInfo GetModInfo(int64_t modID, const KProfileID& id = KProfileIDs::NullProfileID) const override;
		virtual FileInfo GetFileInfo(int64_t modID, int64_t fileID, const KProfileID& id = KProfileIDs::NullProfileID) const override;
		virtual FileInfo::Vector GetFilesList(int64_t modID, const KProfileID& id = KProfileIDs::NullProfileID) const override;
		virtual DownloadInfo::Vector GetFileDownloadLinks(int64_t modID, int64_t fileID, const KProfileID& id = KProfileIDs::NullProfileID) const override;
		virtual EndorsedInfo EndorseMod(int64_t modID, EndorsementState::Value state = EndorsementState::Endorse, const KProfileID& id = KProfileIDs::NullProfileID) override;

		ValidationInfo GetValidationInfo(const wxString& apiKey = wxEmptyString) const;
		GameInfo GetGameInfo(const KProfileID& id = KProfileIDs::NullProfileID) const;
		GameInfo::Vector GetGamesList() const;
		IssueInfo::Vector GetIssues() const;

		wxString ConstructNXM(const FileInfo& fileInfo, const KProfileID& id = KProfileIDs::NullProfileID) const;
};
