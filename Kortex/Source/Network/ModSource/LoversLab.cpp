#include "stdafx.h"
#include <Kortex/NetworkManager.hpp>
#include "LoversLab.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxCURL.h>

namespace Kortex::NetworkManager
{
	LoversLabSource::LoversLabSource()
	{
	}

	wxString LoversLabSource::GetAPIURL() const
	{
		return "https://www.loverslab.com/api";
	}

	KImageEnum LoversLabSource::GetIcon() const
	{
		return KIMG_SITE_LOVERSLAB;
	}
	wxString LoversLabSource::GetName() const
	{
		return "LoversLab";
	}
	wxString LoversLabSource::GetGameID(const GameID& id) const
	{
		return IModSource::GetGameID(id);
	}
	wxString LoversLabSource::GetModURLBasePart(const GameID& id) const
	{
		return "https://www.loverslab.com/files/file";
	}
	wxString LoversLabSource::GetModURL(const ModRepositoryRequest& request)
	{
		return ConstructIPBModURL(request.GetModID().GetValue(), request.GetExtraInfo<wxString>());
	}
}
