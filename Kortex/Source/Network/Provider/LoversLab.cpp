#include "stdafx.h"
#include <Kortex/NetworkManager.hpp>
#include "LoversLab.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxCURL.h>

namespace Kortex::NetworkManager
{
	LoversLabProvider::LoversLabProvider()
		:INetworkProvider(wxS("LoversLab"))
	{
	}

	wxString LoversLabProvider::GetAPIURL() const
	{
		return "https://www.loverslab.com/api";
	}

	bool LoversLabProvider::DoAuthenticate(wxWindow* window)
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
	bool LoversLabProvider::DoValidateAuth(wxWindow* window)
	{
		return HasAuthInfo();
	}
	bool LoversLabProvider::DoSignOut(wxWindow* window)
	{
		return INetworkProvider::DoSignOut(window);
	}
	bool LoversLabProvider::DoIsAuthenticated() const
	{
		return INetworkProvider::DoIsAuthenticated();
	}

	KImageEnum LoversLabProvider::GetIcon() const
	{
		return KIMG_SITE_LOVERSLAB;
	}
	wxString LoversLabProvider::GetName() const
	{
		return "LoversLab";
	}
	wxString LoversLabProvider::GetGameID(const GameID& id) const
	{
		return INetworkProvider::GetGameID(id);
	}
	wxString LoversLabProvider::GetModURLBasePart(const GameID& id) const
	{
		return "https://www.loverslab.com/files/file";
	}
	wxString LoversLabProvider::GetModURL(const ProviderRequest& request)
	{
		return ConstructIPBModURL(request.GetModID().GetValue(), request.GetExtraInfo<wxString>());
	}

	std::unique_ptr<IModInfo> LoversLabProvider::GetModInfo(const ProviderRequest& request) const
	{
		return nullptr;
	}
	std::unique_ptr<IModFileInfo> LoversLabProvider::GetFileInfo(const ProviderRequest& request) const
	{
		return nullptr;
	}
	IModFileInfo::Vector LoversLabProvider::GetFilesList(const ProviderRequest& request) const
	{
		return {};
	}
	IModDownloadInfo::Vector LoversLabProvider::GetFileDownloadLinks(const ProviderRequest& request) const
	{
		return {};
	}
	std::unique_ptr<IModEndorsementInfo> LoversLabProvider::EndorseMod(const ProviderRequest& request, ModEndorsement state)
	{
		return nullptr;
	}
}
