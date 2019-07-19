#pragma once
#include "stdafx.h"
#include "Network/IDownloadManager.h"

namespace Kortex::DownloadManager
{
	class DefaultDownloadManager: public IDownloadManager
	{
		private:
			DownloadItem::RefVector m_Queue;

		private:
			DownloadItem* CheckQueue();
			bool TryStartDownload();

			void DefaultDownloadManager::OnNeedToStartDownload(DownloadEvent& event);
			void DefaultDownloadManager::OnDownloadRemoved(DownloadEvent& event);

		protected:
			void OnInit() override;
			void OnExit() override;
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			
			KWorkspace* CreateWorkspace(KMainWindow* mainWindow) override;

		public:
			std::unique_ptr<IDownloadExecutor> NewDownloadExecutor(DownloadItem& item,
																   const KxURI& uri,
																   const wxString& localPath
			) override;
			bool QueueDownload(ModNetworkRepository& modRepository,
							   const ModDownloadReply& downloadInfo,
							   const ModFileReply& fileInfo,
							   const GameID& id = {}
			) override;
	};
}
