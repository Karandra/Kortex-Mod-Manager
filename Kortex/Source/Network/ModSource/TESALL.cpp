#include "stdafx.h"
#include <Kortex/NetworkManager.hpp>
#include "TESALL.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxHTML.h>

namespace Kortex::NetworkManager
{
	TESALLSource::TESALLSource()
	{
	}

	KImageEnum TESALLSource::GetIcon() const
	{
		return KIMG_SITE_TESALL;
	}
	wxString TESALLSource::GetName() const
	{
		return "TESALL.RU";
	}
	wxString TESALLSource::GetGameID(const GameID& id) const
	{
		return IModSource::GetGameID(id);
	}
	wxString TESALLSource::GetModURLBasePart(const GameID& id) const
	{
		return "http://tesall.ru/files/file";
	}
	wxString TESALLSource::GetModURL(const ModRepositoryRequest& request)
	{
		return ConstructIPBModURL(request.GetModID().GetValue(), request.GetExtraInfo<wxString>());
	}
}
