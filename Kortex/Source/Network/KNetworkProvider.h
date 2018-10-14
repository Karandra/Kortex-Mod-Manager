#pragma once
#include "stdafx.h"
#include "KNetworkConstants.h"
#include "KNetworkProviderModInfo.h"
#include "GameInstance/KGameID.h"
#include "KImageProvider.h"
#include <KxFramework/KxSecretStore.h>
#include <KxFramework/KxVersion.h>
class KNetwork;

class KNetworkProvider
{
	friend class KNetwork;

	public:
		using ModInfo = KNetworkProviderModInfo::ModInfo;
		using FileInfo = KNetworkProviderModInfo::FileInfo;
		using DownloadInfo = KNetworkProviderModInfo::DownloadInfo;
		using EndorsementState = KNetworkProviderModInfo::EndorsementState;
		using EndorsedInfo = KNetworkProviderModInfo::EndorsedInfo;

	public:
		static KImageEnum GetGenericIcon();

	private:
		KxSecretDefaultStoreService m_LoginStore;
		KNetworkProviderID m_TypeID = KNETWORK_PROVIDER_ID_INVALID;
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
		KNetworkProvider(KNetworkProviderID providerID, const wxString& name);
		virtual ~KNetworkProvider();

	public:
		KNetworkProviderID GetID() const
		{
			return m_TypeID;
		}
		bool IsDefault() const;

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

		virtual KImageEnum GetIcon() const = 0;
		virtual wxString GetName() const = 0;
		virtual wxString GetGameID(const KGameID& id = KGameIDs::NullGameID) const = 0;
		virtual wxString& ConvertDescriptionToHTML(wxString& description) const
		{
			return description;
		}
		virtual wxString GetModURLBasePart(const KGameID& id = KGameIDs::NullGameID) const = 0;
		virtual wxString GetModURL(KNetworkModID modID, const wxString& modSignature = wxEmptyString, const KGameID& id = KGameIDs::NullGameID) = 0;

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

		virtual ModInfo GetModInfo(KNetworkModID modID, const KGameID& id = KGameIDs::NullGameID) const = 0;
		virtual FileInfo GetFileInfo(KNetworkModID modID, KNetworkFileID fileID, const KGameID& id = KGameIDs::NullGameID) const = 0;
		virtual FileInfo::Vector GetFilesList(KNetworkModID modID, const KGameID& id = KGameIDs::NullGameID) const = 0;
		virtual DownloadInfo::Vector GetFileDownloadLinks(KNetworkModID modID, KNetworkFileID fileID, const KGameID& id = KGameIDs::NullGameID) const = 0;
		virtual EndorsedInfo EndorseMod(KNetworkModID modID, EndorsementState::Value state = EndorsementState::Endorse, const KGameID& id = KGameIDs::NullGameID) = 0;
};
