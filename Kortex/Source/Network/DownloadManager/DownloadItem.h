#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/ModRepositoryReply.h"
#include "IDownloadItem.h"
#include "IDownloadExecutor.h"
#include "GameInstance/GameID.h"
#include <KxFramework/KxFileItem.h>
#include <KxFramework/KxOptionSet.h>

namespace Kortex
{
	class IGameInstance;
	class IGameMod;
	class IModNetwork;
	class IDownloadManager;
	class IDownloadExecutor;
	class DownloadItemBuilder;
	class ModNetworkRepository;
}

namespace Kortex
{
	class DownloadItem final: public IDownloadItem
	{
		friend class IDownloadManager;
		friend class DownloadItemBuilder;

		public:
			using Vector = std::vector<std::unique_ptr<DownloadItem>>;
			using RefVector = std::vector<DownloadItem*>;

		private:
			// Download
			ModFileReply m_FileInfo;
			ModDownloadReply m_DownloadInfo;
			
			int64_t m_DownloadedSize = 0;
			wxDateTime m_DownloadDate;
			bool m_IsHidden = false;

			// Source
			GameID m_TargetGame;
			IModNetwork* m_ModNetwork = nullptr;

			// Executor
			std::unique_ptr<IDownloadExecutor> m_Executor;
			bool m_IsFailed = false;
			bool m_ShouldResume = false;

		private:
			bool Serialize(wxOutputStream& stream) const;
			bool Deserialize(wxInputStream& stream);

		public:
			DownloadItem() = default;
			DownloadItem(const ModDownloadReply& downloadInfo,
						 const ModFileReply& fileInfo,
						 ModNetworkRepository& modRepository,
						 const GameID& id = {});
			DownloadItem(const DownloadItem&) = delete;

		public:
			bool IsOK() const;
			wxString GetFullPath() const;
			
			const ModFileReply& GetFileInfo() const
			{
				return m_FileInfo;
			}
			const ModDownloadReply& GetDownloadInfo() const
			{
				return m_DownloadInfo;
			}

			wxDateTime GetDownloadDate() const
			{
				return m_DownloadDate;
			}
			int64_t GetDownloadedSize() const
			{
				return m_DownloadedSize;
			}
			int64_t GetTotalSize() const
			{
				return m_FileInfo.Size;
			}

			GameID GetTargetGame() const
			{
				return m_TargetGame;
			}
			void SetTargetGame(const GameID& id)
			{
				m_TargetGame = id;
			}

			const IGameMod* GetMod() const;
			bool IsInstalled() const;

			IModNetwork* GetModNetwork() const
			{
				return m_ModNetwork;
			}
			ModNetworkRepository* GetModRepository() const;
			bool HasModRepository() const
			{
				return m_ModNetwork != nullptr;
			}
			void SetModRepository(ModNetworkRepository& modRepository);
			
			bool IsHidden() const
			{
				return m_IsHidden && !IsRunning();
			}
			void SetHidden(bool value)
			{
				m_IsHidden = value;
			}

			bool CanVisitSource() const;
			bool CanQueryInfo() const;
			bool QueryInfo();

			void LoadDefault(const KxFileItem& fileItem);
			bool Load(const KxFileItem& fileItem);
			bool Save() const;

		public:
			IDownloadExecutor* GetExecutor() const
			{
				return m_Executor.get();
			}
			std::unique_ptr<IDownloadExecutor> OnExecutorDone() override;
			void OnUpdateProgress() override;

			bool IsCompleted() const
			{
				return !IsFailed() && !IsPaused() && !IsRunning();
			}
			bool IsRunning() const
			{
				return m_Executor && m_Executor->IsRunning();
			}
			bool IsPaused() const
			{
				return m_Executor && m_Executor->IsPaused();
			}
			bool IsFailed() const
			{
				return m_IsFailed;
			}
			
			bool CanStart() const;
			bool Start(int64_t startAt = 0);
			bool Stop();

			bool Pause();
			bool Resume();

		public:
			DownloadItem& operator=(const DownloadItem&) = delete;
	};
}
