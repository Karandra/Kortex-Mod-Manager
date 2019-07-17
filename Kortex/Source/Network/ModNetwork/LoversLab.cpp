#include "stdafx.h"
#include <Kortex/NetworkManager.hpp>
#include "LoversLab.h"

namespace Kortex::NetworkManager
{
	LoversLabModNetwork::LoversLabModNetwork()
	{
	}

	wxString LoversLabModNetwork::GetAPIURL() const
	{
		return wxS("https://www.loverslab.com/api");
	}

	ResourceID LoversLabModNetwork::GetIcon() const
	{
		return ImageResourceID::ModNetwork_LoversLab;
	}
	wxString LoversLabModNetwork::GetName() const
	{
		return wxS("LoversLab");
	}

	KxURI LoversLabModNetwork::GetModPageBaseURI(const GameID& id) const
	{
		return wxS("https://www.loverslab.com/files/file");
	}
	KxURI LoversLabModNetwork::GetModPageURI(const ModRepositoryRequest& request) const
	{
		return GetIPBModPageURI(request.GetModID(), request.GetExtraInfo<wxString>());
	}
}
