#pragma once
#include "stdafx.h"
#include "Common.h"
#include <KxFramework/KxQueryInterface.h>
#include <KxFramework/KxSecretStore.h>
#include <optional>

namespace Kortex
{
	class IAuthenticableModNetwork: public KxRTTI::IInterface<IAuthenticableModNetwork>
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
				return const_cast<IAuthenticableModNetwork*>(this)->GetSecretStore();
			}
			
			wxBitmap DownloadSmallBitmap(const wxString& url) const;

		public:
			std::optional<Credentials> ShowCredentialsDialog(wxWindow* parent = nullptr) const;
			std::optional<Credentials> LoadCredentials() const;
			bool SaveCredentials(const Credentials& credentials);

			bool HasUserPicture() const
			{
				return m_UserPicture.IsOk();
			}
			wxBitmap GetUserPicture() const
			{
				return m_UserPicture;
			}
			void SetUserPicture(const wxBitmap& picture)
			{
				m_UserPicture = picture;
				m_UserPicture.SaveFile(GetUserPictureFile(), wxBITMAP_TYPE_PNG);
			}
			wxString GetUserPictureFile() const;

		public:
			virtual bool IsAuthenticated() const = 0;
			virtual bool Authenticate() = 0;
			virtual bool ValidateAuth() = 0;
			virtual bool SignOut() = 0;
	};
}
