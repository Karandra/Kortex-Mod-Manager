#pragma once
#include "stdafx.h"
#include "GameInstance/GameID.h"
#include "DownloadManager/DownloadItem.h"
#include "DownloadManager/DownloadEvent.h"
#include <KxFramework/KxURI.h>
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxComponentSystem.h>

namespace Kortex
{
	class IModNetwork;
	class ModNetworkRepository;
}

namespace Kortex
{
	namespace DownloadManager::Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	}

	class IDownloadManager:
		public ManagerWithTypeInfo<IPluggableManager, DownloadManager::Internal::TypeInfo>,
		public KxSingletonPtr<IDownloadManager>,
		public KxComponentContainer
	{
		public:
			enum class LocationStatus
			{
				Success = 0,
				NotExist,
				NotSpecified,
				InsufficientVolumeSpace,
				InsufficientVolumeCapabilities,
			};

		public:
			static wxString RenameIncrement(const wxString& name);

			static bool IsAssociatedWithLink(const wxString& type);
			static void AssociateWithLink(const wxString& type);

		protected:
			DownloadItem::Vector m_Downloads;
			
		private:
			wxString m_Location;
			int m_MaxConcurrentDownloads = -1;
			bool m_ShowHiddenDownloads = true;

		protected:
			void OnInit() override;
			void OnExit() override;
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			
		protected:
			LocationStatus CheckDownloadLocation(const wxString& directoryPath, int64_t fileSize = -1) const;
			LocationStatus OnAccessDownloadLocation(int64_t fileSize = -1) const;

		public:
			IDownloadManager();
			virtual ~IDownloadManager();

		public:
			DownloadItem::RefVector GetDownloads() const;
			DownloadItem::RefVector GetInactiveDownloads(bool installedOnly = false) const;
			size_t GetActiveDownloadsCount() const;

			DownloadItem& AddDownload(std::unique_ptr<DownloadItem> download);
			bool RemoveDownload(DownloadItem& download);

			void LoadDownloads();
			void SaveDownloads();
			void PauseAllActive();

			bool ShouldShowHiddenDownloads() const
			{
				return m_ShowHiddenDownloads;
			}
			void ShowHiddenDownloads(bool show = true);
			void ToggleHiddenDownloads()
			{
				ShowHiddenDownloads(!m_ShowHiddenDownloads);
			}

			wxString GetDownloadsLocation() const
			{
				return m_Location;
			}
			void SetDownloadsLocation(const wxString& location);
			
			bool HasConcurrentDownloadsLimit() const
			{
				return m_MaxConcurrentDownloads >= 0;
			}
			int GetMaxConcurrentDownloads() const
			{
				return m_MaxConcurrentDownloads;
			}
			void SetMaxConcurrentDownloads(int count);

			DownloadItem* FindDownloadByFileName(const wxString& name, const DownloadItem* except = nullptr) const;
			void AutoRenameIncrement(DownloadItem& item) const;

		public:
			virtual std::unique_ptr<IDownloadExecutor> NewDownloadExecutor(DownloadItem& item,
																		   const KxURI& uri,
																		   const wxString& localPath
			) = 0;
			virtual bool QueueDownload(ModNetworkRepository& modRepository,
									   const ModDownloadReply& downloadInfo,
									   const ModFileReply& fileInfo,
									   const GameID& id = {}
			) = 0;
			virtual bool QueueSimpleDownload(const KxURI& uri, const wxString& localPath = {}) = 0;
			virtual bool QueueUnknownDownload(const wxString& link);
	};
}
