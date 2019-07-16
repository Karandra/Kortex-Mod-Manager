#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include <KxFramework/KxURI.h>

namespace Kortex
{
	class IDownloadManager;
	class DownloadItem;
	class ModNetworkRepository;
	class GameID;
}

namespace Kortex
{
	class DownloadItemBuilder final
	{
		private:
			IDownloadManager* m_DownloadManager = nullptr;
			std::unique_ptr<DownloadItem> m_Item;

		public:
			DownloadItemBuilder();

		public:
			bool IsOK() const;
			explicit operator bool() const
			{
				return IsOK();
			}
			bool operator!() const
			{
				return !IsOK();
			}

			DownloadItem* Commit();
			DownloadItem* Save();

		public:
			DownloadItemBuilder& SetTargetGame(const GameID& id);
			DownloadItemBuilder& SetModRepository(ModNetworkRepository& modRepository);
			DownloadItemBuilder& SetDownloadDate(const wxDateTime& date);
			DownloadItemBuilder& SetURI(const KxURI& value);

			DownloadItemBuilder& SetTotalSize(int64_t size);
			DownloadItemBuilder& SetDownloadedSize(int64_t size);
			
			DownloadItemBuilder& SetName(const wxString& value);
			DownloadItemBuilder& SetDisplayName(const wxString& value);
			DownloadItemBuilder& SetVersion(const KxVersion& value);
			DownloadItemBuilder& SetModID(ModID modID);
			DownloadItemBuilder& SetFileID(ModFileID fileID);
			
			DownloadItemBuilder& SetHidden(bool value = true);
			DownloadItemBuilder& SetResumePos(int64_t pos);
	};
}
