#include "stdafx.h"
#include "ModNetworkAuth.h"
#include "IModNetwork.h"
#include "INetworkManager.h"
#include "UI/KMainWindow.h"
#include "Utility/KAux.h"
#include "Utility/KBitmapSize.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxCredentialsDialog.h>
#include <KxFramework/KxCURL.h>

namespace
{
	using Credentials = Kortex::ModNetworkAuth::Credentials;
}

namespace Kortex
{
	KxStandardID ModNetworkAuth::OnAuthSuccess()
	{
		KxTaskDialog dialog(GetInvokingWindow(), KxID_NONE, KTrf("NetworkManager.AuthSuccess", GetContainer().GetName()));
		dialog.SetMainIcon(KxICON_INFO);
		return static_cast<KxStandardID>(dialog.ShowModal());
	}
	KxStandardID ModNetworkAuth::OnAuthFail()
	{
		KxTaskDialog dialog(GetInvokingWindow(), KxID_NONE, KTrf("NetworkManager.AuthFail", GetContainer().GetName()));
		dialog.SetMainIcon(KxICON_ERROR);
		return static_cast<KxStandardID>(dialog.ShowModal());
	}

	wxBitmap ModNetworkAuth::DownloadSmallBitmap(const wxString& address) const
	{
		auto connection = INetworkManager::GetInstance()->NewCURLSession(address);
		KxCURLBinaryReply reply = connection->Download();
		wxMemoryInputStream stream(reply.GetData(), reply.GetSize());

		wxSize size = KBitmapSize().FromSystemSmallIcon().GetSize();
		return wxBitmap(KAux::ScaleImageAspect(wxImage(stream), -1, size.GetHeight()), 32);
	}

	std::optional<Credentials> ModNetworkAuth::ShowCredentialsDialog(wxWindow* parent) const
	{
		KxCredentialsDialog dialog(parent, KxID_NONE, KTrf("NetworkManager.AuthCaption", GetContainer().GetName()), KTr("NetworkManager.AuthMessage"));
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
	std::optional<Credentials> ModNetworkAuth::LoadCredentials() const
	{
		wxString userName;
		KxSecretValue password;
		if (GetSecretStore().Load(userName, password))
		{
			return Credentials(userName, std::move(password));
		}
		return std::nullopt;
	}
	bool ModNetworkAuth::SaveCredentials(const Credentials& credentials)
	{
		return GetSecretStore().Save(credentials.UserID, credentials.Password);
	}

	wxString ModNetworkAuth::GetUserPictureFile() const
	{
		return GetContainer().GetCacheFolder() + wxS("\\UserPicture.png");
	}
}
