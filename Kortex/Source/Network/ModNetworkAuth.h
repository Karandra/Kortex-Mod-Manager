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

			size_t m_AuthSuccessCount = 0;
			size_t m_AuthFailCount = 0;
			size_t m_AuthResetCount = 0;

		public:
			virtual wxWindow* GetInvokingWindow() const = 0;
			virtual void OnAuthSuccess();
			virtual void OnAuthFail();
			virtual void OnAuthReset();

			virtual KxSecretDefaultStoreService& GetCredentialsStore() = 0;
			const KxSecretDefaultStoreService& GetCredentialsStore() const
			{
				return const_cast<ModNetworkAuth*>(this)->GetCredentialsStore();
			}
			
			wxBitmap DownloadSmallBitmap(const wxString& address) const;

		public:
			std::optional<Credentials> ShowCredentialsDialog(wxWindow* parent = nullptr) const;
			std::optional<Credentials> LoadCredentials() const;
			bool SaveCredentials(const Credentials& credentials);

			void SetUserPicture(const wxBitmap& userPicture);
			bool LoadUserPicture();
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
			virtual void Authenticate() = 0;
			virtual void ValidateAuth() = 0;
			virtual void SignOut() = 0;
	};
}
