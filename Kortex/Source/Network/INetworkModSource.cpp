#include "stdafx.h"
#include "INetworkModSource.h"
#include <Kortex/NetworkManager.hpp>
#include "UI/KMainWindow.h"
#include "Utility/KAux.h"
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxCredentialsDialog.h>

namespace Kortex
{
	KImageEnum INetworkModSource::GetGenericIcon()
	{
		return KIMG_SITE_UNKNOWN;
	}

	void INetworkModSource::Init()
	{
		KxFile(GetCacheFolder()).CreateFolder();
		m_UserPicture.LoadFile(GetUserPictureFile(), wxBITMAP_TYPE_ANY);
	}

	void INetworkModSource::OnAuthSuccess(wxWindow* window)
	{
		KxTaskDialog dialog(window ? window : KMainWindow::GetInstance(), KxID_NONE, KTrf("Network.AuthSuccess", GetName()));
		dialog.SetMainIcon(KxICON_INFO);
		dialog.ShowModal();
	}
	void INetworkModSource::OnAuthFail(wxWindow* window)
	{
		KxTaskDialog dialog(window ? window : KMainWindow::GetInstance(), KxID_NONE, KTrf("Network.AuthFail", GetName()));
		dialog.SetMainIcon(KxICON_ERROR);
		dialog.ShowModal();
	}
	wxString INetworkModSource::ConstructIPBModURL(int64_t modID, const wxString& modSignature) const
	{
		return wxString::Format("%s/%lld-%s", GetModURLBasePart(), modID, modSignature.IsEmpty() ? "x" : modSignature);
	}
	wxBitmap INetworkModSource::DownloadSmallBitmap(const wxString& url) const
	{
		KxCURLSession connection(url);
		KxCURLBinaryReply reply = connection.Download();
		wxMemoryInputStream stream(reply.GetData(), reply.GetSize());

		wxSize size = KGetImageList()->GetSize();
		return wxBitmap(KAux::ScaleImageAspect(wxImage(stream), -1, size.GetHeight()), 32);
	}

	bool INetworkModSource::DoIsAuthenticated() const
	{
		return !m_RequiresAuthentication;
	}
	bool INetworkModSource::DoSignOut(wxWindow* window)
	{
		return m_LoginStore.Delete();
	}

	INetworkModSource::INetworkModSource(const wxString& name)
		:m_LoginStore(wxS("Kortex/") + name)
	{
	}
	INetworkModSource::~INetworkModSource()
	{
	}

	bool INetworkModSource::IsDefault() const
	{
		return this == INetworkManager::GetInstance()->GetDefaultModSource();
	}

	wxString INetworkModSource::GetCacheFolder() const
	{
		return INetworkManager::GetInstance()->GetCacheFolder() + '\\' + KAux::MakeSafeFileName(GetName());
	}
	wxString INetworkModSource::GetUserPictureFile() const
	{
		return GetCacheFolder() + "\\UserPicture.png";
	}

	wxString INetworkModSource::GetGameID(const GameID& id) const
	{
		return id.IsOK() ? id : INetworkManager::GetInstance()->GetConfig().GetNexusID();
	}

	bool INetworkModSource::HasAuthInfo() const
	{
		wxString userName;
		KxSecretValue password;
		return LoadAuthInfo(userName, password) && !userName.IsEmpty() && password.IsOk();
	}
	bool INetworkModSource::LoadAuthInfo(wxString& userName, KxSecretValue& password) const
	{
		return m_LoginStore.Load(userName, password);
	}
	bool INetworkModSource::SaveAuthInfo(const wxString& userName, const KxSecretValue& password)
	{
		return m_LoginStore.Save(userName, password);
	}
	bool INetworkModSource::RequestAuthInfo(wxString& userName, KxSecretValue& password, wxWindow* window, bool* cancelled) const
	{
		KxCredentialsDialog dialog(window, KxID_NONE, KTrf("Network.AuthCaption", GetName()), KTr("Network.AuthMessage"));
		if (dialog.ShowModal() == KxID_OK)
		{
			KxUtility::SetIfNotNull(cancelled, false);
			userName = dialog.GetUserName();
			return !userName.IsEmpty() && dialog.GetPassword(password);
		}

		KxUtility::SetIfNotNull(cancelled, true);
		return false;
	}
	bool INetworkModSource::RequestAuthInfoAndSave(wxWindow* window, bool* cancelled)
	{
		wxString userName;
		KxSecretValue password;
		if (RequestAuthInfo(userName, password, window, cancelled))
		{
			return m_LoginStore.Save(userName, password);
		}
		return false;
	}

	bool INetworkModSource::IsAuthenticated() const
	{
		return DoIsAuthenticated();
	}
	bool INetworkModSource::Authenticate(wxWindow* window)
	{
		bool authOK = DoAuthenticate(window);
		m_RequiresAuthentication = !authOK;
		return authOK;
	}
	bool INetworkModSource::ValidateAuth(wxWindow* window)
	{
		bool authOK = DoValidateAuth(window);
		m_RequiresAuthentication = !authOK;
		return authOK;
	}
	bool INetworkModSource::SignOut(wxWindow* window)
	{
		m_RequiresAuthentication = true;
		return DoSignOut(window);
	}
}
