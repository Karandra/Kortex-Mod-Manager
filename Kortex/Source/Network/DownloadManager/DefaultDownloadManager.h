#pragma once
#include "stdafx.h"
#include "Network/IDownloadManager.h"

namespace Kortex::DownloadManager
{
	class DefaultDownloadManager: public IDownloadManager
	{
		private:
			DownloadItem::Vector m_Downloads;
			bool m_IsReady = false;

		private:
			KWorkspace* CreateWorkspace(KMainWindow* mainWindow) override;
			void SetReady(bool value = true)
			{
				m_IsReady = true;
			}

			void OnDownloadEvent(const DownloadItem& item, ItemEvent eventType) override;

		protected:
			virtual void OnInit() override;
			virtual void OnExit() override;
			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode);

		public:
			DownloadItem::Vector& GetDownloads() override
			{
				return m_Downloads;
			}
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
