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

	KImageEnum LoversLabModNetwork::GetIcon() const
	{
		return KIMG_SITE_LOVERSLAB;
	}
	wxString LoversLabModNetwork::GetName() const
	{
		return wxS("LoversLab");
	}

	wxString LoversLabModNetwork::GetModPageBaseURL(const GameID& id) const
	{
		return wxS("https://www.loverslab.com/files/file");
	}
	wxString LoversLabModNetwork::GetModPageURL(const ModRepositoryRequest& request)
	{
		return GetIPBModPageURL(request.GetModID(), request.GetExtraInfo<wxString>());
	}
}
