#pragma once
#include "stdafx.h"
#include "NetworkConstants.h"
#include "NetworkProviderReply.h"
#include "GameInstance/GameID.h"
#include "KImageProvider.h"
#include <KxFramework/KxSecretStore.h>
#include <KxFramework/KxVersion.h>

namespace Kortex
{
	class INetworkProvider
	{
		friend class INetworkManager;

		public:
			using Vector = std::vector<std::unique_ptr<INetworkProvider>>;
			using RefVector = std::vector<INetworkProvider*>;

		public:
			static KImageEnum GetGenericIcon();

		private:
			template<class T> static std::unique_ptr<T> Create(Network::ProviderID id)
			{
				auto provider = std::make_unique<T>();
				provider->SetID(id);
				provider->Init();
				return provider;
			}

		private:
			KxSecretDefaultStoreService m_LoginStore;
			wxBitmap m_UserPicture;
			Network::ProviderID m_ID = Network::ProviderIDs::Invalid;
			bool m_RequiresAuthentication = true;

		private:
			void SetID(Network::ProviderID id)
			{
				m_ID = id;
			}

		protected:
			void OnAuthSuccess(wxWindow* window = nullptr);
			void OnAuthFail(wxWindow* window = nullptr);
			wxString ConstructIPBModURL(int64_t modID, const wxString& modSignature = wxEmptyString) const;
			wxBitmap DownloadSmallBitmap(const wxString& url) const;

		protected:
			virtual void Init();

			virtual bool DoIsAuthenticated() const;
			virtual bool DoAuthenticate(wxWindow* window = nullptr) = 0;
			virtual bool DoValidateAuth(wxWindow* window = nullptr) = 0;
			virtual bool DoSignOut(wxWindow* window = nullptr) = 0;

		public:
			INetworkProvider(const wxString& name);
			virtual ~INetworkProvider();

		public:
			Network::ProviderID GetID() const
			{
				return m_ID;
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
			virtual wxString GetGameID(const GameID& id = GameIDs::NullGameID) const = 0;
			virtual wxString& ConvertDescriptionToHTML(wxString& description) const
			{
				return description;
			}
			virtual wxString GetModURLBasePart(const GameID& id = GameIDs::NullGameID) const = 0;
			virtual wxString GetModURL(Network::ModID modID, const wxString& modSignature = wxEmptyString, const GameID& id = GameIDs::NullGameID) = 0;

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
			bool RequestAuthInfo(wxString& userName, KxSecretValue& password, wxWindow* window = nullptr, bool* cancelled = nullptr) const;
			bool RequestAuthInfoAndSave(wxWindow* window = nullptr, bool* cancelled = nullptr);

			bool IsAuthenticated() const;
			bool Authenticate(wxWindow* window = nullptr);
			bool ValidateAuth(wxWindow* window = nullptr);
			bool SignOut(wxWindow* window = nullptr);

			virtual Network::ModInfo GetModInfo(Network::ModID modID, const GameID& id = GameIDs::NullGameID) const = 0;
			virtual Network::FileInfo GetFileItem(Network::ModID modID, Network::FileID fileID, const GameID& id = GameIDs::NullGameID) const = 0;
			virtual Network::FileInfo::Vector GetFilesList(Network::ModID modID, const GameID& id = GameIDs::NullGameID) const = 0;
			virtual Network::DownloadInfo::Vector GetFileDownloadLinks(Network::ModID modID, Network::FileID fileID, const GameID& id = GameIDs::NullGameID) const = 0;
			virtual Network::EndorsementInfo EndorseMod(Network::ModID modID, Network::EndorsementState::Value state = Network::EndorsementState::Endorse, const GameID& id = GameIDs::NullGameID) = 0;
	};
}
