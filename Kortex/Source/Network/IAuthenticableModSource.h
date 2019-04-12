#pragma once
#include "stdafx.h"
#include "Common.h"
#include <KxFramework/KxQueryInterface.h>
#include <KxFramework/KxSecretStore.h>
#include <optional>

namespace Kortex
{
	class IAuthenticableModSource: public KxRTTI::IInterface<IAuthenticableModSource>
	{
		public:
			struct Credentials
			{
				wxString UserID;
				KxSecretValue Password;

				Credentials(const wxString& userID, KxSecretValue&& password)
					:UserID(userID), Password(std::move(password))
				{
				}
				Credentials(const Credentials&) = delete;
				Credentials(Credentials&&) = default;
			};

		protected:
			virtual wxWindow* GetInvokingWindow() const = 0;
			virtual KxStandardID OnAuthSuccess();
			virtual KxStandardID OnAuthFail();

			virtual KxSecretDefaultStoreService& GetSecretStore() = 0;
			KxSecretDefaultStoreService& GetSecretStore() const
			{
				return const_cast<IAuthenticableModSource*>(this)->GetSecretStore();
			}
		
		public:
			std::optional<Credentials> ShowCredentialsDialog(wxWindow* parent = nullptr) const;
			std::optional<Credentials> LoadCredentials() const;
			bool SaveCredentials(const Credentials& credentials);

		public:
			virtual bool IsAuthenticated() const = 0;
			virtual bool Authenticate() = 0;
			virtual bool ValidateAuth() = 0;
			virtual bool SignOut() = 0;
	};
}
