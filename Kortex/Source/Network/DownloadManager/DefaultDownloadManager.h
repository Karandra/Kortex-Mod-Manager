#pragma once
#include "stdafx.h"
#include "Network/IDownloadManager.h"

namespace Kortex::DownloadManager
{
	class DefaultDownloadManager: public IDownloadManager
	{
		protected:
			void OnInit() override;
			void OnExit() override;
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			
			KWorkspace* CreateWorkspace(KMainWindow* mainWindow) override;

		public:
			void LoadDownloads() override;
			void SaveDownloads() override;

			bool ShouldShowHiddenDownloads() const override;
			void ShowHiddenDownloads(bool show = true) override;

			wxString GetDownloadsLocation() const override;
			void SetDownloadsLocation(const wxString& location) override;
			LocationStatus OnAccessDownloadLocation(int64_t fileSize = -1) const override;

			std::unique_ptr<IDownloadExecutor> NewDownloadExecutor(DownloadItem& item,
																   const KxURI& url,
																   const wxString& localPath
			) override;
			bool QueueDownload(ModNetworkRepository& modRepository,
							   const ModDownloadReply& downloadInfo,
							   const ModFileReply& fileInfo,
							   const GameID& id = {}
			) override;
	};
}
