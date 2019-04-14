#include "stdafx.h"
#include <Kortex/NetworkManager.hpp>
#include "TESALL.h"

namespace Kortex::NetworkManager
{
	TESALLModNetwork::TESALLModNetwork()
	{
	}

	KImageEnum TESALLModNetwork::GetIcon() const
	{
		return KIMG_SITE_TESALL;
	}
	wxString TESALLModNetwork::GetName() const
	{
		return wxS("TESALL.RU");
	}

	wxString TESALLModNetwork::GetModPageBaseURL(const GameID& id) const
	{
		return wxS("http://tesall.ru/files/file");
	}
	wxString TESALLModNetwork::GetModPageURL(const ModRepositoryRequest& request)
	{
		return GetIPBModPageURL(request.GetModID(), request.GetExtraInfo<wxString>());
	}
}
