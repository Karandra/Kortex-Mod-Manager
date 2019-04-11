#include "stdafx.h"
#include "IModSource.h"
#include <Kortex/NetworkManager.hpp>
#include "UI/KMainWindow.h"
#include "Utility/KAux.h"
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxCredentialsDialog.h>

namespace Kortex
{
	KImageEnum IModSource::GetGenericIcon()
	{
		return KIMG_SITE_UNKNOWN;
	}

	void IModSource::Init()
	{
		KxFile(GetCacheFolder()).CreateFolder();
		m_UserPicture.LoadFile(GetUserPictureFile(), wxBITMAP_TYPE_ANY);
	}

	void IModSource::OnAuthSuccess(wxWindow* window)
	{
		KxTaskDialog dialog(window ? window : KMainWindow::GetInstance(), KxID_NONE, KTrf("Network.AuthSuccess", GetName()));
		dialog.SetMainIcon(KxICON_INFO);
		dialog.ShowModal();
	}
	void IModSource::OnAuthFail(wxWindow* window)
	{
		KxTaskDialog dialog(window ? window : KMainWindow::GetInstance(), KxID_NONE, KTrf("Network.AuthFail", GetName()));
		dialog.SetMainIcon(KxICON_ERROR);
		dialog.ShowModal();
	}
	wxString IModSource::ConstructIPBModURL(int64_t modID, const wxString& modSignature) const
	{
		return wxString::Format("%s/%lld-%s", GetModURLBasePart(), modID, modSignature.IsEmpty() ? "x" : modSignature);
	}
	wxBitmap IModSource::DownloadSmallBitmap(const wxString& url) const
	{
		KxCURLSession connection(url);
		KxCURLBinaryReply reply = connection.Download();
		wxMemoryInputStream stream(reply.GetData(), reply.GetSize());

		wxSize size = KGetImageList()->GetSize();
		return wxBitmap(KAux::ScaleImageAspect(wxImage(stream), -1, size.GetHeight()), 32);
	}

	bool IModSource::DoIsAuthenticated() const
	{
		return !m_RequiresAuthentication;
	}
	bool IModSource::DoSignOut(wxWindow* window)
	{
		return m_LoginStore.Delete();
	}

	IModSource::IModSource(const wxString& name)
		:m_LoginStore(wxS("Kortex/") + name)
	{
	}
	IModSource::~IModSource()
	{
	}

	bool IModSource::IsDefault() const
	{
		return this == INetworkManager::GetInstance()->GetDefaultModSource();
	}

	wxString IModSource::GetCacheFolder() const
	{
		return INetworkManager::GetInstance()->GetCacheFolder() + '\\' + KAux::MakeSafeFileName(GetName());
	}
	wxString IModSource::GetUserPictureFile() const
	{
		return GetCacheFolder() + "\\UserPicture.png";
	}

	wxString IModSource::GetGameID(const GameID& id) const
	{
		return id.IsOK() ? id : INetworkManager::GetInstance()->GetConfig().GetNexusID();
	}

	bool IModSource::HasAuthInfo() const
	{
		wxString userName;
		KxSecretValue password;
		return LoadAuthInfo(userName, password) && !userName.IsEmpty() && password.IsOk();
	}
	bool IModSource::LoadAuthInfo(wxString& userName, KxSecretValue& password) const
	{
		return m_LoginStore.Load(userName, password);
	}
	bool IModSource::SaveAuthInfo(const wxString& userName, const KxSecretValue& password)
	{
		return m_LoginStore.Save(userName, password);
	}
	bool IModSource::RequestAuthInfo(wxString& userName, KxSecretValue& password, wxWindow* window, bool* cancelled) const
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
	bool IModSource::RequestAuthInfoAndSave(wxWindow* window, bool* cancelled)
	{
		wxString userName;
		KxSecretValue password;
		if (RequestAuthInfo(userName, password, window, cancelled))
		{
			return m_LoginStore.Save(userName, password);
		}
		return false;
	}

	bool IModSource::IsAuthenticated() const
	{
		return DoIsAuthenticated();
	}
	bool IModSource::Authenticate(wxWindow* window)
	{
		bool authOK = DoAuthenticate(window);
		m_RequiresAuthentication = !authOK;
		return authOK;
	}
	bool IModSource::ValidateAuth(wxWindow* window)
	{
		bool authOK = DoValidateAuth(window);
		m_RequiresAuthentication = !authOK;
		return authOK;
	}
	bool IModSource::SignOut(wxWindow* window)
	{
		m_RequiresAuthentication = true;
		return DoSignOut(window);
	}
}
