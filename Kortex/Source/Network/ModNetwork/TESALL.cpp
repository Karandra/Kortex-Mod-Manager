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

	KxURI TESALLModNetwork::GetModPageBaseURI(const GameID& id) const
	{
		return wxS("http://tesall.ru/files/file");
	}
	KxURI TESALLModNetwork::GetModPageURI(const ModRepositoryRequest& request)
	{
		return GetIPBModPageURI(request.GetModID(), request.GetExtraInfo<wxString>());
	}
}
