#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/INetworkProvider.h"
#include "Network/NetworkProviderReply.h"
#include "Network/NetworkProviderReplyStructs.h"

namespace Kortex::NetworkManager
{
	class NexusProvider;
}

namespace Kortex::Nexus::Internal::ReplyStructs
{
	struct ValidationInfo
	{
		wxString UserName;
		wxString APIKey;
		wxString EMail;
		wxString ProfilePicture;
		int64_t UserID = -1;
		bool IsPremium = false;
		bool IsSupporter = false;
	};
	struct GameInfo
	{
		int64_t ID = -1;
		wxString Name;
		wxString Genre;
		wxString ForumURL;
		wxString NexusURL;
		wxString DomainName;

		int64_t FilesCount = -1;
		int64_t DownloadsCount = -1;
		int64_t ModsCount = -1;
		wxDateTime m_ApprovedDate;
	};
	struct ModDownloadInfoNXM
	{
		wxString Key;
		wxString Expires;
	};
}

namespace Kortex
{
	class INexusModDownloadInfo: public RTTI::IExtendInterface<INexusModDownloadInfo, IModDownloadInfo>
	{
		public:
			virtual wxString GetKey() const = 0;
			virtual wxString GetExpires() const = 0;
	};
}

namespace Kortex::Nexus
{
	class ModFileInfo: public IModFileInfo
	{
		friend class NetworkManager::NexusProvider;

		private:
			NetworkManager::ReplyStructs::ModFileInfo m_Data;

		public:
			bool IsOK() const override
			{
				return !m_Data.Name.IsEmpty() && m_Data.Size > 0 && m_Data.ID.GetValue() > 0 && m_Data.ModID.GetValue() > 0;
			}
			std::unique_ptr<IModFileInfo> Clone() const override
			{
				return std::make_unique<ModFileInfo>(*this);
			}

		public:
			ModID GetModID() const override
			{
				return m_Data.ModID;
			}
			void SetModID(ModID value) override
			{
				m_Data.ModID = value;
			}

			ModFileID GetID() const override
			{
				return m_Data.ID;
			}
			void SetID(ModFileID value) override
			{
				m_Data.ID = value;
			}

			int64_t GetSize() const override
			{
				return m_Data.Size;
			}
			void SetSize(int64_t value) override
			{
				m_Data.Size = value;
			}
			
			bool IsPrimary() const override
			{
				return m_Data.IsPrimary;
			}
			
			wxString GetName() const override
			{
				return m_Data.Name;
			}
			void SetName(const wxString& value) override
			{
				m_Data.Name = value;
			}
			
			wxString GetDisplayName() const override
			{
				return m_Data.DisplayName.IsEmpty() ? m_Data.Name : m_Data.DisplayName;
			}
			void SetDisplayName(const wxString& value) override
			{
				m_Data.DisplayName = value;
			}

			KxVersion GetVersion() const override
			{
				return m_Data.Version;
			}
			void SetVersion(const KxVersion& value) override
			{
				m_Data.Version = value;
			}

			wxString GetChangeLog() const override
			{
				return m_Data.ChangeLog;
			}
			void SetChangeLog(const wxString& value)
			{
				m_Data.ChangeLog = value;
			}

			wxDateTime GetUploadDate() const override
			{
				return m_Data.UploadDate;
			}
			CategoryID GetCategory() const override
			{
				return m_Data.Category;
			}
	};

	class ModInfo: public IModInfo
	{
		friend class NetworkManager::NexusProvider;

		private:
			NetworkManager::ReplyStructs::ModInfo m_Data;
			std::unique_ptr<IModFileInfo> m_PrimaryFile;

		public:
			bool IsOK() const override
			{
				return m_Data.ID.GetValue() > 0 && !m_Data.Name.IsEmpty();
			}
			std::unique_ptr<IModInfo> Clone() const override
			{
				auto clone = std::make_unique<ModInfo>();
				clone->m_Data = m_Data;
				clone->m_PrimaryFile = m_PrimaryFile->Clone();
				
				return clone;
			}

		public:
			ModID GetID() const override
			{
				return m_Data.ID;
			}
			
			wxString GetName() const override
			{
				return m_Data.Name;
			}
			wxString GetSummary() const override
			{
				if (m_Data.m_Summary.IsEmpty())
				{
					return m_Data.m_Description.Left(128) + wxS("...");
				}
				return m_Data.m_Summary;
			}
			wxString GetDescription() const override
			{
				return m_Data.m_Description;
			}
			
			wxString GetAuthor() const override
			{
				return m_Data.Author.IsEmpty() ? m_Data.Uploader : m_Data.Author;
			}
			wxString GetUploader() const override
			{
				return m_Data.Uploader;
			}
			wxString GetUploaderProfile() const override
			{
				return m_Data.UploaderProfile;
			}
			wxString GetMainImage() const override
			{
				return m_Data.MainImage;
			}
			
			KxVersion GetVersion() const override
			{
				return m_Data.Version;
			}
			wxDateTime GetUploadDate() const override
			{
				return m_Data.UploadDate;
			}
			wxDateTime GetLastUpdateDate() const override
			{
				return m_Data.LastUpdateDate;
			}

			IModFileInfo* GetPrimaryFile() override
			{
				return m_PrimaryFile.get();
			}
			ModEndorsement GetEndorsementState() const
			{
				return m_Data.EndorsementState;
			}
	
			bool ContainsAdultContent() const override
			{
				return m_Data.ContainsAdultContent;
			}
	};

	class ModDownloadInfo: public INexusModDownloadInfo
	{
		friend class NetworkManager::NexusProvider;

		private:
			NetworkManager::ReplyStructs::ModDownloadInfo m_Data;
			Internal::ReplyStructs::ModDownloadInfoNXM m_NXM;

		public:
			bool IsOK() const override
			{
				return !m_Data.URL.IsEmpty();
			}
			std::unique_ptr<IModDownloadInfo> Clone() const override
			{
				return std::make_unique<ModDownloadInfo>(*this);
			}

		public:
			wxString GetName() const override
			{
				return m_Data.Name.IsEmpty() ? m_Data.URL : m_Data.Name;
			}
			wxString GetShortName() const override
			{
				return m_Data.ShortName.IsEmpty() ? GetName() : m_Data.ShortName;
			}
			
			wxString GetURL() const override
			{
				return m_Data.URL;
			}
			void SetURL(const wxString& value) override
			{
				m_Data.URL = value;
			}
	
			wxString GetKey() const override
			{
				return m_NXM.Key;
			}
			wxString GetExpires() const override
			{
				return m_NXM.Expires;
			}
	};

	class ModEndorsementInfo: public IModEndorsementInfo
	{
		friend class NetworkManager::NexusProvider;

		private:
			NetworkManager::ReplyStructs::ModEndorsementInfo m_Data;

		public:
			bool IsOK() const override
			{
				return true;
			}
			std::unique_ptr<IModEndorsementInfo> Clone() const override
			{
				return std::make_unique<ModEndorsementInfo>(*this);
			}
	
		public:
			ModEndorsement GetEndorsement() const override
			{
				return m_Data.Endorsement;
			}
			wxString GetMessage() const override
			{
				return m_Data.Message;
			}
	};
}

namespace Kortex::Nexus
{
	class ValidationInfo: public NetworkManager::BaseModReply
	{
		friend class NetworkManager::NexusProvider;

		private:
			Internal::ReplyStructs::ValidationInfo m_Data;

		public:
			bool IsOK() const override
			{
				return m_Data.UserID > 0;
			}

		public:
			int64_t GetUserID() const
			{
				return m_Data.UserID;
			}
			wxString GetUserName() const
			{
				return m_Data.UserName;
			}
			wxString GetEMail() const
			{
				return m_Data.EMail;
			}

			wxString GetAPIKey() const
			{
				return m_Data.APIKey;
			}
			wxString GetProfilePicture() const
			{
				return m_Data.ProfilePicture;
			}

			bool IsPremium() const
			{
				return m_Data.IsPremium;
			}
			bool IsSupporter() const
			{
				return m_Data.IsSupporter;
			}
	};

	class GameInfo: public NetworkManager::BaseModReply
	{
		friend class NetworkManager::NexusProvider;

		public:
			using Vector = std::vector<std::unique_ptr<GameInfo>>;

		private:
			Internal::ReplyStructs::GameInfo m_Data;

		public:
			bool IsOK() const override
			{
				return m_Data.ID > 0 && m_Data.FilesCount >= 0 && m_Data.DownloadsCount >= 0 && m_Data.ModsCount >= 0 && m_Data.m_ApprovedDate.IsValid();
			}

		public:
			int64_t GetID() const
			{
				return m_Data.ID;
			}
			wxString GetName() const
			{
				return m_Data.Name;
			}
			wxString GetGenre() const
			{
				return m_Data.Genre;
			}
			
			wxString GetForumURL() const
			{
				return m_Data.ForumURL;
			}
			wxString GetNexusURL() const
			{
				return m_Data.NexusURL;
			}
			wxString GetDomainName() const
			{
				return m_Data.DomainName;
			}
			wxDateTime GetApprovedDate() const
			{
				return m_Data.m_ApprovedDate;
			}

			int64_t GetFilesCount() const
			{
				return m_Data.FilesCount;
			}
			int64_t GetDownloadsCount() const
			{
				return m_Data.DownloadsCount;
			}
			int64_t GetModsCount() const
			{
				return m_Data.ModsCount;
			}
	};

	class IssueInfo: public NetworkManager::BaseModReply
	{
		friend class NetworkManager::NexusProvider;

		public:
			using Vector = std::vector<std::unique_ptr<IssueInfo>>;

		private:
			// TBD
			
		public:
			bool IsOK() const override
			{
				return false;
			}
	};
}
