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
			static void ConfigureCommandLine(wxCmdLineParser& parser);
			static KxURI GetLinkFromCommandLine(const wxCmdLineParser& parser);

			static bool IsAssociatedWithLink(const wxString& type);
			static void AssociateWithLink(const wxString& type);

		protected:
			DownloadItem::Vector m_Downloads;
			
		protected:
			LocationStatus CheckDownloadLocation(const wxString& directoryPath, int64_t fileSize = -1) const;

		public:
			IDownloadManager();
			virtual ~IDownloadManager();

		public:
			DownloadItem::RefVector GetDownloads() const;
			
			virtual void LoadDownloads() = 0;
			virtual void SaveDownloads() = 0;
			void PauseAllActive();

			virtual bool ShouldShowHiddenDownloads() const = 0;
			virtual void ShowHiddenDownloads(bool show = true) = 0;
			void ToggleHiddenDownloads()
			{
				ShowHiddenDownloads(!ShouldShowHiddenDownloads());
			}

			virtual wxString GetDownloadsLocation() const = 0;
			virtual void SetDownloadsLocation(const wxString& location) = 0;
			virtual LocationStatus OnAccessDownloadLocation(int64_t fileSize = -1) const = 0;
			
			DownloadItem::RefVector GetInactiveDownloads(bool installedOnly = false) const;
			DownloadItem* FindDownloadByFileName(const wxString& name, const DownloadItem* except = nullptr) const;
			void AutoRenameIncrement(DownloadItem& entry) const;

			DownloadItem& AddDownload(std::unique_ptr<DownloadItem> download);
			bool RemoveDownload(DownloadItem& download);

			virtual std::unique_ptr<IDownloadExecutor> NewDownloadExecutor(DownloadItem& item,
																		   const KxURI& uri,
																		   const wxString& localPath
			) = 0;
			virtual bool QueueDownload(ModNetworkRepository& modRepository,
									   const ModDownloadReply& downloadInfo,
									   const ModFileReply& fileInfo,
									   const GameID& id = {}
			) = 0;
			virtual bool TryQueueDownloadLink(const KxURI& link);
	};
}
