#include "stdafx.h"
#include <Kortex/NetworkManager.hpp>
#include "TESALL.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxHTML.h>

namespace Kortex::NetworkManager
{
	bool TESALLProvider::DoAuthenticate(wxWindow* window)
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
	bool TESALLProvider::DoValidateAuth(wxWindow* window)
	{
		return HasAuthInfo();
	}
	bool TESALLProvider::DoSignOut(wxWindow* window)
	{
		return INetworkProvider::DoSignOut(window);
	}
	bool TESALLProvider::DoIsAuthenticated() const
	{
		return INetworkProvider::DoIsAuthenticated();
	}

	TESALLProvider::TESALLProvider()
		:INetworkProvider(wxS("TESALL"))
	{
	}

	KImageEnum TESALLProvider::GetIcon() const
	{
		return KIMG_SITE_TESALL;
	}
	wxString TESALLProvider::GetName() const
	{
		return "TESALL.RU";
	}
	wxString TESALLProvider::GetGameID(const GameID& id) const
	{
		return INetworkProvider::GetGameID(id);
	}
	wxString TESALLProvider::GetModURLBasePart(const GameID& id) const
	{
		return "http://tesall.ru/files/file";
	}
	wxString TESALLProvider::GetModURL(ModID modID, const wxString& modSignature, const GameID& id)
	{
		return ConstructIPBModURL(modID.GetValue(), modSignature);
	}
}
