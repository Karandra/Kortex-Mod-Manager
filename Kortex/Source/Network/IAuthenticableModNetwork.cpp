#include "stdafx.h"
#include "IAuthenticableModNetwork.h"
#include "IModNetwork.h"
#include "INetworkManager.h"
#include "UI/KMainWindow.h"
#include "Utility/KAux.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxCredentialsDialog.h>
#include <KxFramework/KxCURL.h>

namespace
{
	using Credentials = Kortex::IAuthenticableModNetwork::Credentials;
}

namespace Kortex
{
	KxStandardID IAuthenticableModNetwork::OnAuthSuccess()
	{
		KxTaskDialog dialog(GetInvokingWindow(), KxID_NONE, KTrf("NetworkManager.AuthSuccess", QueryInterface<IModNetwork>()->GetName()));
		dialog.SetMainIcon(KxICON_INFO);
		return static_cast<KxStandardID>(dialog.ShowModal());
	}
	KxStandardID IAuthenticableModNetwork::OnAuthFail()
	{
		KxTaskDialog dialog(GetInvokingWindow(), KxID_NONE, KTrf("NetworkManager.AuthFail", QueryInterface<IModNetwork>()->GetName()));
		dialog.SetMainIcon(KxICON_ERROR);
		return static_cast<KxStandardID>(dialog.ShowModal());
	}

	wxBitmap IAuthenticableModNetwork::DownloadSmallBitmap(const wxString& address) const
	{
		auto connection = INetworkManager::GetInstance()->NewCURLSession(address);
		KxCURLBinaryReply reply = connection->Download();
		wxMemoryInputStream stream(reply.GetData(), reply.GetSize());

		wxSize size = KGetImageList()->GetSize();
		return wxBitmap(KAux::ScaleImageAspect(wxImage(stream), -1, size.GetHeight()), 32);
	}

	std::optional<Credentials> IAuthenticableModNetwork::ShowCredentialsDialog(wxWindow* parent) const
	{
		KxCredentialsDialog dialog(parent, KxID_NONE, KTrf("NetworkManager.AuthCaption", QueryInterface<IModNetwork>()->GetName()), KTr("NetworkManager.AuthMessage"));
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
	std::optional<Credentials> IAuthenticableModNetwork::LoadCredentials() const
	{
		wxString userName;
		KxSecretValue password;
		if (GetSecretStore().Load(userName, password))
		{
			return Credentials(userName, std::move(password));
		}
		return std::nullopt;
	}
	bool IAuthenticableModNetwork::SaveCredentials(const Credentials& credentials)
	{
		return GetSecretStore().Save(credentials.UserID, credentials.Password);
	}

	wxString IAuthenticableModNetwork::GetUserPictureFile() const
	{
		return QueryInterface<IModNetwork>()->GetCacheFolder() + wxS("\\UserPicture.png");
	}
}
