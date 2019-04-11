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
		return IModSource::DoSignOut(window);
	}
	bool TESALLProvider::DoIsAuthenticated() const
	{
		return IModSource::DoIsAuthenticated();
	}

	TESALLProvider::TESALLProvider()
		:IModSource(wxS("TESALL"))
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
		return IModSource::GetGameID(id);
	}
	wxString TESALLProvider::GetModURLBasePart(const GameID& id) const
	{
		return "http://tesall.ru/files/file";
	}
	wxString TESALLProvider::GetModURL(const ProviderRequest& request)
	{
		return ConstructIPBModURL(request.GetModID().GetValue(), request.GetExtraInfo<wxString>());
	}

	std::unique_ptr<IModInfo> TESALLProvider::GetModInfo(const ProviderRequest& request) const
	{
		return nullptr;
	}
	std::unique_ptr<IModFileInfo> TESALLProvider::GetFileInfo(const ProviderRequest& request) const
	{
		return nullptr;
	}
	IModFileInfo::Vector TESALLProvider::GetFilesList(const ProviderRequest& request) const
	{
		return {};
	}
	IModDownloadInfo::Vector TESALLProvider::GetFileDownloadLinks(const ProviderRequest& request) const
	{
		return {};
	}
	std::unique_ptr<IModEndorsementInfo> TESALLProvider::EndorseMod(const ProviderRequest& request, ModEndorsement state)
	{
		return nullptr;
	}
}
