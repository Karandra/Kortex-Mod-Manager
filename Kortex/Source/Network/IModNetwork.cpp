#include "stdafx.h"
#include "IModNetwork.h"
#include <Kortex/NetworkManager.hpp>
#include "GameMods/IGameMod.h"
#include "Utility/Common.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxCredentialsDialog.h>

namespace Kortex
{
	ResourceID IModNetwork::GetGenericIcon()
	{
		return ImageResourceID::ModNetwork_Unknown;
	}

	void IModNetwork::DoOnInit()
	{
		KxFile(GetCacheDirectory()).CreateFolder();
		OnInit();
	}
	void IModNetwork::DoOnExit()
	{
		OnExit();
		RemoveAllComponents();
	}

	KxURI IModNetwork::GetIPBModPageURI(ModID modID, const wxString& modSignature) const
	{
		return KxString::Format(wxS("%1/%2-%3"), GetModPageBaseURI().BuildUnescapedURI(), modID.GetValue(), modSignature.IsEmpty() ? wxS("x") : modSignature);
	}

	bool IModNetwork::IsDefault() const
	{
		return this == INetworkManager::GetInstance()->GetDefaultModNetwork();
	}
	wxString IModNetwork::GetCacheDirectory() const
	{
		return INetworkManager::GetInstance()->GetCacheDirectory() + wxS('\\') + Utility::MakeSafeFileName(GetName());
	}
	wxString IModNetwork::GetLocationInCache(const wxString& relativePath) const
	{
		return GetCacheDirectory() + wxS('\\') + relativePath;
	}

	KxURI IModNetwork::GetModPageURI(const IGameMod& mod) const
	{
		if (const ModSourceItem* item = mod.GetModSourceStore().GetItem(*this))
		{
			ModRepositoryRequest request(item->GetModInfo());
			request.SetExtraInfo(mod.GetSignature());
			return GetModPageURI(request);
		}
		return {};
	}
	KxURI IModNetwork::GetModPageURI(const DownloadItem& download) const
	{
		if (download.IsOK())
		{
			return GetModPageURI(download.GetNetworkModInfo());
		}
		return {};
	}
}
