#include "stdafx.h"
#include <Kortex/NetworkManager.hpp>
#include "TESALL.h"

namespace Kortex::NetworkManager
{
	TESALLModNetwork::TESALLModNetwork()
	{
	}

	ResourceID TESALLModNetwork::GetIcon() const
	{
		return ImageResourceID::ModNetwork_TESALL;
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
