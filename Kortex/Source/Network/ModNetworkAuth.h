#pragma once
#include "stdafx.h"
#include "Common.h"
#include "IModNetwork.h"
#include <KxFramework/KxComponentSystem.h>
#include <KxFramework/KxSecretStore.h>
#include <optional>

namespace Kortex
{
	class ModNetworkAuth: public KxComponentOf<IModNetwork>
	{
		public:
			struct Credentials
			{
				public:
					wxString UserID;
					KxSecretValue Password;

				public:
					Credentials(const wxString& userID, KxSecretValue&& password)
						:UserID(userID), Password(std::move(password))
					{
					}
					Credentials(const Credentials&) = delete;
					Credentials(Credentials&&) = default;

				public:
					Credentials& operator=(const Credentials&) = delete;
					Credentials& operator=(Credentials&&) = default;
			};

		private:
			wxBitmap m_UserPicture;

		protected:
			virtual wxWindow* GetInvokingWindow() const = 0;
			virtual KxStandardID OnAuthSuccess();
			virtual KxStandardID OnAuthFail();

			virtual KxSecretDefaultStoreService& GetSecretStore() = 0;
			const KxSecretDefaultStoreService& GetSecretStore() const
			{
				return const_cast<ModNetworkAuth*>(this)->GetSecretStore();
			}
			
			wxBitmap DownloadSmallBitmap(const wxString& address) const;

		public:
			std::optional<Credentials> ShowCredentialsDialog(wxWindow* parent = nullptr) const;
			std::optional<Credentials> LoadCredentials() const;
			bool SaveCredentials(const Credentials& credentials);

			void SetUserPicture(const wxBitmap& userPicture)
			{
				m_UserPicture = userPicture;
				m_UserPicture.SaveFile(GetUserPictureFile(), wxBITMAP_TYPE_PNG);
			}
			bool LoadUserPicture()
			{
				return m_UserPicture.LoadFile(GetUserPictureFile(), wxBITMAP_TYPE_ANY);
			}
			wxString GetUserPictureFile() const;
			bool HasUserPicture() const
			{
				return m_UserPicture.IsOk();
			}
			wxBitmap GetUserPicture() const
			{
				return m_UserPicture;
			}

		public:
			virtual bool IsAuthenticated() const = 0;
			virtual bool Authenticate() = 0;
			virtual bool ValidateAuth() = 0;
			virtual bool SignOut() = 0;
	};
}
