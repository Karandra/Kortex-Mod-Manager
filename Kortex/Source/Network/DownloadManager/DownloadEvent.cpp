#include "stdafx.h"
#include "DownloadEvent.h"
#include "Network/IDownloadManager.h"

namespace Kortex
{
	bool DownloadEvent::ShouldShowHidden() const
	{
		return IDownloadManager::GetInstance()->ShouldShowHiddenDownloads();
	}
}
