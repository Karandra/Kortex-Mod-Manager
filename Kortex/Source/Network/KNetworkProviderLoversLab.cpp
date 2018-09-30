#include "stdafx.h"
#include "KNetwork.h"
#include "KNetworkProviderLoversLab.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxCURL.h>

KNetworkProviderLoversLab::KNetworkProviderLoversLab(KNetworkProviderID providerID)
	:KNetworkProvider(providerID, wxS("LoversLab"))
{
}

wxString KNetworkProviderLoversLab::GetAPIURL() const
{
	return "https://www.loverslab.com/api";
}

bool KNetworkProviderLoversLab::DoAuthenticate(wxWindow* window)
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
bool KNetworkProviderLoversLab::DoValidateAuth(wxWindow* window)
{
	return HasAuthInfo();
}
bool KNetworkProviderLoversLab::DoSignOut(wxWindow* window)
{
	return KNetworkProvider::DoSignOut(window);
}
bool KNetworkProviderLoversLab::DoIsAuthenticated() const
{
	return KNetworkProvider::DoIsAuthenticated();
}

KImageEnum KNetworkProviderLoversLab::GetIcon() const
{
	return KIMG_SITE_LOVERSLAB;
}
wxString KNetworkProviderLoversLab::GetName() const
{
	return "LoversLab";
}
wxString KNetworkProviderLoversLab::GetGameID(const KProfileID& id) const
{
	return KNetworkProvider::GetGameID(id);
}
wxString KNetworkProviderLoversLab::GetModURLBasePart(const KProfileID& id) const
{
	return "https://www.loverslab.com/files/file";
}
wxString KNetworkProviderLoversLab::GetModURL(int64_t modID, const wxString& modSignature, const KProfileID& id)
{
	return ConstructIPBModURL(modID, modSignature);
}

KNetworkProvider::ModInfo KNetworkProviderLoversLab::GetModInfo(int64_t modID, const KProfileID& id) const
{
	return ModInfo();
}
KNetworkProvider::FileInfo KNetworkProviderLoversLab::GetFileInfo(int64_t modID, int64_t fileID, const KProfileID& id) const
{
	return FileInfo();
}
KNetworkProvider::FileInfo::Vector KNetworkProviderLoversLab::GetFilesList(int64_t modID, const KProfileID& id) const
{
	return FileInfo::Vector();
}
KNetworkProvider::DownloadInfo::Vector KNetworkProviderLoversLab::GetFileDownloadLinks(int64_t modID, int64_t fileID, const KProfileID& id) const
{
	return DownloadInfo::Vector();
}
KNetworkProvider::EndorsedInfo KNetworkProviderLoversLab::EndorseMod(int64_t modID, EndorsementState::Value state, const KProfileID& id)
{
	return EndorsedInfo();
}
