#include "stdafx.h"
#include "KNetworkProvider.h"
#include "KNetwork.h"
#include "UI/KMainWindow.h"
#include "GameInstance/Config/KNetworkConfig.h"
#include "KAux.h"
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxCredentialsDialog.h>

KImageEnum KNetworkProvider::GetGenericIcon()
{
	return KIMG_SITE_UNKNOWN;
}

void KNetworkProvider::Init()
{
	KxFile(GetCacheFolder()).CreateFolder();
	m_UserPicture.LoadFile(GetUserPictureFile(), wxBITMAP_TYPE_ANY);
}

void KNetworkProvider::OnAuthSuccess(wxWindow* window)
{
	KxTaskDialog dialog(window ? window : KMainWindow::GetInstance(), KxID_NONE, KTrf("Network.AuthSuccess", GetName()));
	dialog.SetMainIcon(KxICON_INFO);
	dialog.ShowModal();
}
void KNetworkProvider::OnAuthFail(wxWindow* window)
{
	KxTaskDialog dialog(window ? window : KMainWindow::GetInstance(), KxID_NONE, KTrf("Network.AuthFail", GetName()));
	dialog.SetMainIcon(KxICON_ERROR);
	dialog.ShowModal();
}
wxString KNetworkProvider::ConstructIPBModURL(int64_t modID, const wxString& modSignature) const
{
	return wxString::Format("%s/%lld-%s", GetModURLBasePart(), modID, modSignature.IsEmpty() ? "x" : modSignature);
}
wxBitmap KNetworkProvider::DownloadSmallBitmap(const wxString& url) const
{
	KxCURLSession connection(url);
	KxCURLBinaryReply reply = connection.Download();
	wxMemoryInputStream stream(reply.GetData(), reply.GetSize());

	wxSize size = KGetImageList()->GetSize();
	return wxBitmap(KAux::ScaleImageAspect(wxImage(stream), -1, size.GetHeight()), 32);
}

bool KNetworkProvider::DoIsAuthenticated() const
{
	return !m_RequiresAuthentication;
}
bool KNetworkProvider::DoSignOut(wxWindow* window)
{
	return m_LoginStore.Delete();
}

KNetworkProvider::KNetworkProvider(KNetworkProviderID providerID, const wxString& name)
	:m_LoginStore(wxS("Kortex/") + name), m_TypeID(providerID)
{
}
KNetworkProvider::~KNetworkProvider()
{
}

bool KNetworkProvider::IsDefault() const
{
	return this == KNetwork::GetInstance()->GetCurrentProvider();
}

wxString KNetworkProvider::GetCacheFolder() const
{
	return KNetwork::GetInstance()->GetCacheFolder() + '\\' + KAux::MakeSafeFileName(GetName());
}
wxString KNetworkProvider::GetUserPictureFile() const
{
	return GetCacheFolder() + "\\UserPicture.png";
}

wxString KNetworkProvider::GetGameID(const KGameID& id) const
{
	return id.IsOK() ? id : KNetworkConfig::GetInstance()->GetNexusID();
}

bool KNetworkProvider::HasAuthInfo() const
{
	wxString userName;
	KxSecretValue password;
	return LoadAuthInfo(userName, password) && !userName.IsEmpty() && password.IsOk();
}
bool KNetworkProvider::LoadAuthInfo(wxString& userName, KxSecretValue& password) const
{
	return m_LoginStore.Load(userName, password);
}
bool KNetworkProvider::SaveAuthInfo(const wxString& userName, const KxSecretValue& password)
{
	return m_LoginStore.Save(userName, password);
}
bool KNetworkProvider::RequestAuthInfo(wxString& userName, KxSecretValue& password, wxWindow* window, bool* cancelled) const
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
bool KNetworkProvider::RequestAuthInfoAndSave(wxWindow* window, bool* cancelled)
{
	wxString userName;
	KxSecretValue password;
	if (RequestAuthInfo(userName, password, window, cancelled))
	{
		return m_LoginStore.Save(userName, password);
	}
	return false;
}

bool KNetworkProvider::IsAuthenticated() const
{
	return DoIsAuthenticated();
}
bool KNetworkProvider::Authenticate(wxWindow* window)
{
	bool authOK = DoAuthenticate(window);
	m_RequiresAuthentication = !authOK;
	return authOK;
}
bool KNetworkProvider::ValidateAuth(wxWindow* window)
{
	bool authOK = DoValidateAuth(window);
	m_RequiresAuthentication = !authOK;
	return authOK;
}
bool KNetworkProvider::SignOut(wxWindow* window)
{
	m_RequiresAuthentication = true;
	return DoSignOut(window);
}
