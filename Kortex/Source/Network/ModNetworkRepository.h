#pragma once
#include "stdafx.h"
#include "Common.h"
#include "IModNetwork.h"
#include "ModRepositoryRequest.h"
#include "ModRepositoryReply.h"
#include "ModRepositoryLimits.h"
#include <KxFramework/KxComponentSystem.h>
#include <KxFramework/KxFileItem.h>
#include <optional>
class KxMenu;

namespace Kortex
{
	class DownloadItem;
}

namespace Kortex
{
	class ModNetworkRepository: public KxComponentOf<IModNetwork>
	{
		public:
			virtual ModRepositoryLimits GetRequestLimits() const = 0;
			virtual void OnDownloadMenu(KxMenu& menu, DownloadItem* download = nullptr) = 0;
			virtual bool QueryDownload(const KxFileItem& fileItem, const DownloadItem& download, ModFileReply& fileReply) = 0;
			
			virtual bool QueueDownload(const wxString& link) = 0;
			virtual wxAny GetDownloadTarget(const wxString& link) = 0;

			virtual std::optional<ModInfoReply> GetModInfo(const ModRepositoryRequest& request) const = 0;
			virtual std::optional<ModEndorsementReply> EndorseMod(const ModRepositoryRequest& request, ModEndorsement state) = 0;
			
			virtual std::optional<ModFileReply> GetModFileInfo(const ModRepositoryRequest& request) const = 0;
			virtual std::vector<ModFileReply> GetModFiles(const ModRepositoryRequest& request) const = 0;
			virtual std::vector<ModDownloadReply> GetFileDownloads(const ModRepositoryRequest& request) const = 0;
	};
}
