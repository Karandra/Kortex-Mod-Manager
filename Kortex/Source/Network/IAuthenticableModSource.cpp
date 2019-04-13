#include "stdafx.h"
#include "IAuthenticableModSource.h"
#include "IModSource.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxCredentialsDialog.h>

namespace
{
	using Credentials = Kortex::IAuthenticableModSource::Credentials;
}

namespace Kortex
{
	KxStandardID IAuthenticableModSource::OnAuthSuccess()
	{
		KxTaskDialog dialog(GetInvokingWindow(), KxID_NONE, KTrf("NetworkManager.AuthSuccess", QueryInterface<IModSource>()->GetName()));
		dialog.SetMainIcon(KxICON_INFO);
		return static_cast<KxStandardID>(dialog.ShowModal());
	}
	KxStandardID IAuthenticableModSource::OnAuthFail()
	{
		KxTaskDialog dialog(GetInvokingWindow(), KxID_NONE, KTrf("NetworkManager.AuthFail", QueryInterface<IModSource>()->GetName()));
		dialog.SetMainIcon(KxICON_ERROR);
		return static_cast<KxStandardID>(dialog.ShowModal());
	}

	std::optional<Credentials> IAuthenticableModSource::ShowCredentialsDialog(wxWindow* parent) const
	{
		KxCredentialsDialog dialog(parent, KxID_NONE, KTrf("NetworkManager.AuthCaption", QueryInterface<IModSource>()->GetName()), KTr("NetworkManager.AuthMessage"));
		if (dialog.ShowModal() == KxID_OK)
		{
			wxString userName = dialog.GetUserName();
			KxSecretValue password;
			if (!userName.IsEmpty() && dialog.GetPassword(password))
			{
				return Credentials(userName, std::move(password));
			}
		}
		return std::nullopt;
	}
	std::optional<Credentials> IAuthenticableModSource::LoadCredentials() const
	{
		wxString userName;
		KxSecretValue password;
		if (GetSecretStore().Load(userName, password))
		{
			return Credentials(userName, std::move(password));
		}
		return std::nullopt;
	}
	bool IAuthenticableModSource::SaveCredentials(const Credentials& credentials)
	{
		return GetSecretStore().Save(credentials.UserID, credentials.Password);
	}
}
