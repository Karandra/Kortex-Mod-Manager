#include "stdafx.h"
#include "KNetwork.h"
#include "KNetworkProviderTESALL.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxHTML.h>

bool KNetworkProviderTESALL::DoAuthenticate(wxWindow* window)
{
	bool cancelled = false;
	if (RequestAuthInfoAndSave(window, &cancelled))
	{
		OnAuthSuccess(window);
		return true;
	}
	else if (!cancelled)
	{
		OnAuthFail(window);
	}
	return false;
}
bool KNetworkProviderTESALL::DoValidateAuth(wxWindow* window)
{
	return HasAuthInfo();
}
bool KNetworkProviderTESALL::DoSignOut(wxWindow* window)
{
	return KNetworkProvider::DoSignOut(window);
}
bool KNetworkProviderTESALL::DoIsAuthenticated() const
{
	return KNetworkProvider::DoIsAuthenticated();
}

KNetworkProviderTESALL::KNetworkProviderTESALL(KNetworkProviderID providerID)
	:KNetworkProvider(providerID, wxS("TESALL"))
{
}

KImageEnum KNetworkProviderTESALL::GetIcon() const
{
	return KIMG_SITE_TESALL;
}
wxString KNetworkProviderTESALL::GetName() const
{
	return "TESALL.RU";
}
wxString KNetworkProviderTESALL::GetGameID(const KGameID& id) const
{
	return KNetworkProvider::GetGameID(id);
}
wxString KNetworkProviderTESALL::GetModURLBasePart(const KGameID& id) const
{
	return "http://tesall.ru/files/file";
}
wxString KNetworkProviderTESALL::GetModURL(int64_t modID, const wxString& modSignature, const KGameID& id)
{
	return ConstructIPBModURL(modID, modSignature);
}

KNetworkProvider::ModInfo KNetworkProviderTESALL::GetModInfo(int64_t modID, const KGameID& id) const
{
	return ModInfo();
}
KNetworkProvider::FileInfo KNetworkProviderTESALL::GetFileInfo(int64_t modID, int64_t fileID, const KGameID& id) const
{
	return FileInfo();
}
KNetworkProvider::FileInfo::Vector KNetworkProviderTESALL::GetFilesList(int64_t modID, const KGameID& id) const
{
	return FileInfo::Vector();
}
KNetworkProvider::DownloadInfo::Vector KNetworkProviderTESALL::GetFileDownloadLinks(int64_t modID, int64_t fileID, const KGameID& id) const
{
	return DownloadInfo::Vector();
}
KNetworkProvider::EndorsedInfo KNetworkProviderTESALL::EndorseMod(int64_t modID, EndorsementState::Value state, const KGameID& id)
{
	return EndorsedInfo();
}
