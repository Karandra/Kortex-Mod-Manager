#include "stdafx.h"
#include "ModNetworkAuth.h"
#include "IModNetwork.h"
#include "INetworkManager.h"
#include "Application/IApplication.h"
#include "Application/INotificationCenter.h"
#include "Utility/BitmapSize.h"
#include "Utility/Drawing.h"
#include <KxFramework/KxCredentialsDialog.h>
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxFile.h>

namespace Kortex
{
	void ModNetworkAuth::OnAuthSuccess()
	{
		INetworkManager::GetInstance()->OnAuthStateChanged();

		if (m_AuthSuccessCount != 0)
		{
			const IApplication* app = IApplication::GetInstance();
			const IModNetwork& modNetwork = GetContainer();
			INotificationCenter::Notify(app->GetName(), KTrf("NetworkManager.AuthSuccess", modNetwork.GetName()), KxICON_INFORMATION);
		}
		m_AuthSuccessCount++;
	}
	void ModNetworkAuth::OnAuthFail()
	{
		m_AuthFailCount++;
		INetworkManager::GetInstance()->OnAuthStateChanged();

		const IApplication* app = IApplication::GetInstance();
		const IModNetwork& modNetwork = GetContainer();
		INotificationCenter::Notify(app->GetName(), KTrf("NetworkManager.AuthFail", modNetwork.GetName()), KxICON_ERROR);
	}
	void ModNetworkAuth::OnAuthReset()
	{
		m_AuthResetCount++;
		INetworkManager::GetInstance()->OnAuthStateChanged();

		const IApplication* app = IApplication::GetInstance();
		const IModNetwork& modNetwork = GetContainer();
		INotificationCenter::Notify(app->GetName(), KTrf("NetworkManager.AuthReset", modNetwork.GetName()), KxICON_WARNING);
	}

	wxBitmap ModNetworkAuth::DownloadSmallBitmap(const wxString& address) const
	{
		auto connection = INetworkManager::GetInstance()->NewCURLSession(address);
		KxCURLBinaryReply reply = connection->Download();
		wxMemoryInputStream stream(reply.GetData(), reply.GetSize());

		wxSize size = Utility::BitmapSize().FromSystemSmallIcon().GetSize();
		return wxBitmap(Utility::Drawing::ScaleImageAspect(wxImage(stream), -1, size.GetHeight()), 32);
	}

	auto ModNetworkAuth::ShowCredentialsDialog(wxWindow* parent) const -> std::optional<Credentials>
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
	auto ModNetworkAuth::LoadCredentials() const -> std::optional<Credentials>
	{
		wxString userName;
		KxSecretValue password;
		if (GetCredentialsStore().Load(userName, password))
		{
			return Credentials(userName, std::move(password));
		}
		return std::nullopt;
	}
	bool ModNetworkAuth::SaveCredentials(const Credentials& credentials)
	{
		return GetCredentialsStore().Save(credentials.UserID, credentials.Password);
	}

	void ModNetworkAuth::SetUserPicture(const wxBitmap& userPicture)
	{
		m_UserPicture = userPicture;
		if (m_UserPicture.IsOk())
		{
			m_UserPicture.SaveFile(GetUserPictureFile(), wxBITMAP_TYPE_PNG);
		}
		else
		{
			KxFile(GetUserPictureFile()).RemoveFile();
		}
	}
	bool ModNetworkAuth::LoadUserPicture()
	{
		return m_UserPicture.LoadFile(GetUserPictureFile(), wxBITMAP_TYPE_ANY);
	}
	wxString ModNetworkAuth::GetUserPictureFile() const
	{
		return GetContainer().GetLocationInCache(wxS("UserPicture.png"));
	}
}
