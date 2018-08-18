#pragma once
#include "stdafx.h"
#include "KNetworkConstants.h"
#include "Profile/KProfileID.h"
#include "KImageProvider.h"
#include <KxFramework/KxSecretStore.h>
#include <KxFramework/KxVersion.h>
class KNetwork;
class KNetworkProviderNexus;

namespace KNetworkProviderNS
{
	class InfoBase
	{
		private:
			bool m_ShouldTryLater = false;

		public:
			virtual ~InfoBase() = default;

		public:
			virtual bool IsOK() const = 0;
			virtual void Reset() = 0;

			bool ShouldTryLater() const
			{
				return m_ShouldTryLater;
			}
			void SetShouldTryLater()
			{
				m_ShouldTryLater = true;
			}
	};
	class EndorsementState
	{
		public:
			enum Value
			{
				Invalid = -1,
				Undecided,
				Endorse,
				Abstain,
			};

		private:
			Value m_State = Value::Invalid;

		public:
			virtual ~EndorsementState() = default;

		public:
			bool IsValid() const
			{
				return m_State != Value::Invalid;
			}

			bool IsEndorsed() const
			{
				return m_State == Value::Endorse;
			}
			bool IsAbstained() const
			{
				return m_State == Value::Abstain;
			}
			bool ShouldRemindEndorse() const
			{
				return !IsEndorsed() && !IsAbstained();
			}
	
			void SetEndorsed()
			{
				m_State = Value::Endorse;
			}
			void SetAbstained()
			{
				m_State = Value::Abstain;
			}
			void SetUndecided()
			{
				m_State = Value::Undecided;
			}
	};

	class FileInfo: public InfoBase
	{
		friend class KNetworkProviderNexus;

		public:
			using Vector = std::vector<FileInfo>;

		private:
			int64_t m_ModID = -1;
			int64_t m_ID = -1;
			int64_t m_Size = -1;
			wxString m_Name;
			wxString m_DisplayName;
			wxString m_ChangeLog;
			KxVersion m_Version;
			wxDateTime m_UploadDate;
			bool m_IsPrimary = false;

		public:
			virtual bool IsOK() const override
			{
				return !m_Name.IsEmpty() && m_Size >= 0 && m_ID >= 0 && m_ModID >= 0;
			}
			virtual void Reset() override
			{
				*this = FileInfo();
			}

		public:
			int64_t GetModID() const
			{
				return m_ModID;
			}
			void SetModID(int64_t id)
			{
				m_ModID = id;
			}

			int64_t GetID() const
			{
				return m_ID;
			}
			void SetID(int64_t id)
			{
				m_ID = id;
			}
			
			int64_t GetSize() const
			{
				return m_Size;
			}
			void SetSize(int64_t size)
			{
				m_Size = size;
			}

			bool IsPrimary() const
			{
				return m_IsPrimary;
			}
			void SetPrimary(bool set = true)
			{
				m_IsPrimary = set;
			}

			const wxString& GetName() const
			{
				return m_Name;
			}
			void SetName(const wxString& name)
			{
				m_Name = name;
			}
			
			wxString GetDisplayName() const
			{
				return m_DisplayName.IsEmpty() ? m_Name : m_DisplayName;
			}
			void SetDisplayName(const wxString& name)
			{
				m_DisplayName = name;
			}
			
			wxDateTime GetUploadDate() const
			{
				return m_UploadDate;
			}
			void SetUploadDate(const wxDateTime& date)
			{
				m_UploadDate = date;
			}
			
			const KxVersion& GetVersion() const
			{
				return m_Version;
			}
			void SetVersion(const KxVersion& version)
			{
				m_Version = version;
			}
			
			bool HasChangeLog() const
			{
				return !m_ChangeLog.IsEmpty();
			}
			const wxString& GetChangeLog() const
			{
				return m_ChangeLog;
			}
			void SetChangeLog(const wxString& changeLog)
			{
				m_ChangeLog = changeLog;
			}
	};
	class ModInfo: public InfoBase, public EndorsementState
	{
		friend class KNetworkProviderNexus;

		private:
			int64_t m_ID = -1;
			wxString m_Name;
			wxString m_Summary;
			wxString m_Description;
			wxString m_Author;
			wxString m_Uploader;
			wxString m_UploaderProfileURL;
			wxString m_MainImageURL;
			KxVersion m_Version;
			wxDateTime m_UploadDate;
			wxDateTime m_LastUpdateDate;
			FileInfo m_PrimaryFile;
			bool m_ContainsAdultContent = false;

		public:
			virtual bool IsOK() const override
			{
				return m_ID >= 0 && !m_Name.IsEmpty();
			}
			virtual void Reset() override
			{
				*this = ModInfo();
			}
	
		public:
			int64_t GetID() const
			{
				return m_ID;
			}
			const wxString& GetName() const
			{
				return m_Name;
			}
			wxString GetSummary() const
			{
				if (m_Summary.IsEmpty())
				{
					return m_Description.Left(128) + "...";
				}
				return m_Summary;
			}
			const wxString& GetDescription() const
			{
				return m_Description;
			}
			const wxString& GetAuthor() const
			{
				return m_Author.IsEmpty() ? m_Uploader : m_Author;
			}
			const wxString& GetUploader() const
			{
				return m_Uploader;
			}
			const wxString& GetUploaderProfileURL() const
			{
				return m_UploaderProfileURL;
			}
			const wxString& GetMainImageURL() const
			{
				return m_MainImageURL;
			}
			const KxVersion& GetVersion() const
			{
				return m_Version;
			}
			wxDateTime GetUploadDate() const
			{
				return m_UploadDate;
			}
			wxDateTime GetLastUpdateDate() const
			{
				return m_LastUpdateDate;
			}
			bool ContainsAdultContent() const
			{
				return m_ContainsAdultContent;
			}
	
			bool HasPrimaryFile() const
			{
				return m_PrimaryFile.IsOK();
			}
			FileInfo& GetPrimaryFile()
			{
				return m_PrimaryFile;
			}
			const FileInfo& GetPrimaryFile() const
			{
				return m_PrimaryFile;
			}
	};
	class DownloadInfo: public InfoBase
	{
		friend class KNetworkProviderNexus;

		public:
			using Vector = std::vector<DownloadInfo>;

		private:
			wxString m_Name;
			wxString m_ShortName;
			wxString m_URL;

		public:
			DownloadInfo(const wxString& url = wxEmptyString, const wxString& name = wxEmptyString, const wxString& shortName = wxEmptyString)
				:m_URL(url), m_Name(name), m_ShortName(shortName)
			{
			}

		public:
			virtual bool IsOK() const override
			{
				return !m_URL.IsEmpty();
			}
			virtual void Reset() override
			{
				*this = DownloadInfo();
			}
	
		public:
			const wxString& GetName() const
			{
				return m_Name.IsEmpty() ? m_URL : m_Name;
			}
			void SetName(const wxString& name)
			{
				m_Name = name;
			}
			
			const wxString& GetShortName() const
			{
				return m_ShortName.IsEmpty() ? GetName() : m_ShortName;
			}
			void SetShortName(const wxString& name)
			{
				m_ShortName = name;
			}
			
			const wxString& GetURL() const
			{
				return m_URL;
			}
			void SetURL(const wxString& url)
			{
				m_URL = url;
			}
	};
	class EndorsedInfo: public InfoBase, public EndorsementState
	{
		friend class KNetworkProviderNexus;

		private:
			wxString m_Message;

		public:
			virtual bool IsOK() const override
			{
				return EndorsementState::IsValid();
			}
			virtual void Reset() override
			{
				*this = EndorsedInfo();
			}
	
		public:
			bool HasMessage() const
			{
				return !m_Message.IsEmpty();
			}
			const wxString& GetMessage() const
			{
				return m_Message;
			}
	};
}

class KNetworkProvider
{
	friend class KNetwork;

	public:
		using ModInfo = KNetworkProviderNS::ModInfo;
		using FileInfo = KNetworkProviderNS::FileInfo;
		using DownloadInfo = KNetworkProviderNS::DownloadInfo;
		using EndorsementState = KNetworkProviderNS::EndorsementState;
		using EndorsedInfo = KNetworkProviderNS::EndorsedInfo;

	public:
		static wxString GetStoreServiceName(KNetworkProviderID providerID);
		static KImageEnum GetGenericIcon();

	private:
		KxSecretDefaultStoreService m_LoginStore;
		wxBitmap m_UserPicture;
		bool m_RequiresAuthentication = true;

	private:
		void Init();

	protected:
		void OnAuthSuccess(wxWindow* window = NULL);
		void OnAuthFail(wxWindow* window = NULL);
		wxString ConstructIPBModURL(int64_t modID, const wxString& modSignature = wxEmptyString) const;
		wxBitmap DownloadSmallBitmap(const wxString& url) const;

	protected:
		virtual bool DoIsAuthenticated() const;
		virtual bool DoAuthenticate(wxWindow* window = NULL) = 0;
		virtual bool DoValidateAuth(wxWindow* window = NULL) = 0;
		virtual bool DoSignOut(wxWindow* window = NULL) = 0;

	public:
		KNetworkProvider(KNetworkProviderID providerID);
		virtual ~KNetworkProvider();

	public:
		bool HasUserPicture() const
		{
			return m_UserPicture.IsOk();
		}
		wxBitmap GetUserPicture() const
		{
			return HasUserPicture() ? m_UserPicture : KGetBitmap(GetIcon());
		}
		void SetUserPicture(const wxBitmap& picture)
		{
			m_UserPicture = picture;
			m_UserPicture.SaveFile(GetUserPictureFile(), wxBITMAP_TYPE_PNG);
		}

		wxString GetCacheFolder() const;
		wxString GetUserPictureFile() const;

		virtual KNetworkProviderID GetID() const = 0;
		virtual KImageEnum GetIcon() const = 0;
		virtual wxString GetName() const = 0;
		virtual wxString GetGameID(const KProfileID& id = KProfileIDs::NullProfileID) const = 0;
		virtual wxString& ConvertDescriptionToHTML(wxString& description) const
		{
			return description;
		}
		virtual wxString GetModURLBasePart(const KProfileID& id = KProfileIDs::NullProfileID) const = 0;
		virtual wxString GetModURL(int64_t modID, const wxString& modSignature = wxEmptyString, const KProfileID& id = KProfileIDs::NullProfileID) = 0;

		bool HasAuthInfo() const;
		bool LoadAuthInfo(wxString& userName, KxSecretValue& password) const;
		bool LoadAuthInfo(wxString& userName) const
		{
			KxSecretValue password;
			return LoadAuthInfo(userName, password);
		}
		bool LoadAuthInfo(KxSecretValue& password) const
		{
			wxString userName;
			return LoadAuthInfo(userName, password);
		}
		bool SaveAuthInfo(const wxString& userName, const KxSecretValue& password);
		bool RequestAuthInfo(wxString& userName, KxSecretValue& password, wxWindow* window = NULL, bool* cancelled = NULL) const;
		bool RequestAuthInfoAndSave(wxWindow* window = NULL, bool* cancelled = NULL);
		
		bool IsAuthenticated() const;
		bool Authenticate(wxWindow* window = NULL);
		bool ValidateAuth(wxWindow* window = NULL);
		bool SignOut(wxWindow* window = NULL);

		virtual ModInfo GetModInfo(int64_t modID, const KProfileID& id = KProfileIDs::NullProfileID) const = 0;
		virtual FileInfo GetFileInfo(int64_t modID, int64_t fileID, const KProfileID& id = KProfileIDs::NullProfileID) const = 0;
		virtual FileInfo::Vector GetFilesList(int64_t modID, const KProfileID& id = KProfileIDs::NullProfileID) const = 0;
		virtual DownloadInfo::Vector GetFileDownloadLinks(int64_t modID, int64_t fileID, const KProfileID& id = KProfileIDs::NullProfileID) const = 0;
		virtual EndorsedInfo EndorseMod(int64_t modID, EndorsementState::Value state = EndorsementState::Endorse, const KProfileID& id = KProfileIDs::NullProfileID) = 0;
};
