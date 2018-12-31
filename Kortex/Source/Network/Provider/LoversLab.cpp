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
	wxString LoversLabProvider::GetModURL(ModID modID, const wxString& modSignature, const GameID& id)
	{
		return ConstructIPBModURL(modID.GetValue(), modSignature);
	}
}
