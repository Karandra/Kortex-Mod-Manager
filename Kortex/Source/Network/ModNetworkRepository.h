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

namespace Kortex
{
	class IDownloadEntry;
}

namespace Kortex
{
	class ModNetworkRepository: public KxComponentOf<IModNetwork>
	{
		public:
			virtual ModRepositoryLimits GetRequestLimits() const = 0;
			virtual bool IsAutomaticUpdateCheckAllowed() const = 0;
			virtual bool RestoreBrokenDownload(const KxFileItem& fileItem, IDownloadEntry& download) = 0;

			virtual std::optional<ModInfoReply> GetModInfo(const ModRepositoryRequest& request) const = 0;
			virtual std::optional<ModEndorsementReply> EndorseMod(const ModRepositoryRequest& request, ModEndorsement state) = 0;
			
			virtual std::optional<ModFileReply> GetModFileInfo(const ModRepositoryRequest& request) const = 0;
			virtual std::vector<ModFileReply> GetModFiles(const ModRepositoryRequest& request) const = 0;
			virtual std::vector<ModDownloadReply> GetFileDownloads(const ModRepositoryRequest& request) const = 0;
	};
}
