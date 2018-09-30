#pragma once
#include "stdafx.h"
#include "KNetworkConstants.h"
#include "KNetworkProviderModInfo.h"

namespace KNetworkProviderNexusModInfo
{
	using InfoBase = KNetworkProviderModInfo::InfoBase;

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
