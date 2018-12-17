#include "stdafx.h"
#include <Kortex/NetworkManager.hpp>
#include "TESALL.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxHTML.h>

namespace Kortex::Network
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
	wxString TESALLProvider::GetModURL(int64_t modID, const wxString& modSignature, const GameID& id)
	{
		return ConstructIPBModURL(modID, modSignature);
	}

	ModInfo TESALLProvider::GetModInfo(int64_t modID, const GameID& id) const
	{
		return ModInfo();
	}
	FileInfo TESALLProvider::GetFileItem(int64_t modID, int64_t fileID, const GameID& id) const
	{
		return FileInfo();
	}
	FileInfo::Vector TESALLProvider::GetFilesList(int64_t modID, const GameID& id) const
	{
		return FileInfo::Vector();
	}
	DownloadInfo::Vector TESALLProvider::GetFileDownloadLinks(int64_t modID, int64_t fileID, const GameID& id) const
	{
		return DownloadInfo::Vector();
	}
	EndorsementInfo TESALLProvider::EndorseMod(int64_t modID, EndorsementState::Value state, const GameID& id)
	{
		return EndorsementInfo();
	}
}
