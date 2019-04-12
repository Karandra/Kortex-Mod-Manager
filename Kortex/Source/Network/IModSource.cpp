#include "stdafx.h"
#include "IModSource.h"
#include <Kortex/NetworkManager.hpp>
#include "UI/KMainWindow.h"
#include "Utility/KAux.h"
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxCredentialsDialog.h>

namespace Kortex
{
	KImageEnum IModSource::GetGenericIcon()
	{
		return KIMG_SITE_UNKNOWN;
	}

	wxString IModSource::ConstructIPBModURL(int64_t modID, const wxString& modSignature) const
	{
		return wxString::Format("%s/%lld-%s", GetModURLBasePart(), modID, modSignature.IsEmpty() ? "x" : modSignature);
	}
	wxBitmap IModSource::DownloadSmallBitmap(const wxString& url) const
	{
		KxCURLSession connection(url);
		KxCURLBinaryReply reply = connection.Download();
		wxMemoryInputStream stream(reply.GetData(), reply.GetSize());

		wxSize size = KGetImageList()->GetSize();
		return wxBitmap(KAux::ScaleImageAspect(wxImage(stream), -1, size.GetHeight()), 32);
	}
	void IModSource::Init()
	{
		KxFile(GetCacheFolder()).CreateFolder();
		m_UserPicture.LoadFile(GetUserPictureFile(), wxBITMAP_TYPE_ANY);
	}

	bool IModSource::IsDefault() const
	{
		return this == INetworkManager::GetInstance()->GetDefaultModSource();
	}

	wxString IModSource::GetCacheFolder() const
	{
		return INetworkManager::GetInstance()->GetCacheFolder() + '\\' + KAux::MakeSafeFileName(GetName());
	}
	wxString IModSource::GetUserPictureFile() const
	{
		return GetCacheFolder() + "\\UserPicture.png";
	}

	wxString IModSource::GetGameID(const GameID& id) const
	{
		return id.IsOK() ? id : INetworkManager::GetInstance()->GetConfig().GetNexusID();
	}
}
