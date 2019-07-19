#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/ModRepositoryReply.h"
#include "Network/NetworkModInfo.h"
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
			int64_t m_DownloadSpeed = 0;
			wxDateTime m_DownloadDate;
			bool m_IsHidden = false;
			bool m_IsWaiting = false;

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

			bool DoStart(int64_t startAt = 0);
			void SetWaiting(bool value = true)
			{
				m_IsWaiting = value;
			}

		protected:
			std::unique_ptr<IDownloadExecutor> OnExecutorEnd() override;
			void OnExecutorProgress() override;

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

			KxURI GetURI() const
			{
				return m_DownloadInfo.URI;
			}
			wxString GetServerName() const
			{
				return !m_DownloadInfo.ShortName.IsEmpty() ? m_DownloadInfo.ShortName : m_DownloadInfo.Name;
			}
			NetworkModInfo GetNetworkModInfo() const
			{
				return NetworkModInfo(m_FileInfo.ModID, m_FileInfo.ID);
			}

			wxDateTime GetDownloadDate() const
			{
				return m_DownloadDate;
			}
			int64_t GetDownloadSpeed() const
			{
				return m_DownloadSpeed;
			}
			int64_t GetDownloadedSize() const
			{
				return m_DownloadedSize;
			}
			int64_t GetTotalSize() const
			{
				return m_FileInfo.Size;
			}

			wxString GetName() const
			{
				return m_FileInfo.Name;
			}
			wxString GetDisplayName() const
			{
				return !m_FileInfo.DisplayName.IsEmpty() ? m_FileInfo.DisplayName : m_FileInfo.Name;
			}
			KxVersion GetVersion() const
			{
				return m_FileInfo.Version;
			}
			
			bool HasChangeLog() const
			{
				return !m_FileInfo.ChangeLog.IsEmpty();
			}
			wxString GetChangeLog() const
			{
				return m_FileInfo.ChangeLog;
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
			bool IsVisible() const
			{
				return !IsHidden();
			}
			void Hide(bool value = true)
			{
				if (!IsRunning())
				{
					m_IsHidden = value;
				}
			}
			void Show(bool value = true)
			{
				Hide(!value);
			}

			bool CanVisitSource() const;
			bool CanQueryInfo() const;
			bool QueryInfo();

			void LoadDefault(const KxFileItem& fileItem);
			bool Load(const KxFileItem& fileItem);
			bool Save() const;

		public:
			bool IsCompleted() const
			{
				return !m_IsWaiting && !IsFailed() && !IsPaused() && !IsRunning();
			}
			bool IsRunning() const
			{
				return m_Executor && m_Executor->IsRunning();
			}
			bool IsPaused() const
			{
				return m_ShouldResume || (m_Executor && m_Executor->IsPaused());
			}
			bool IsFailed() const
			{
				return m_IsFailed;
			}
			bool IsWaiting() const
			{
				return m_IsWaiting;
			}

			bool CanStart() const;
			bool Start();
			bool Stop();

			bool CanResume() const;
			bool Pause();
			bool Resume();

		public:
			DownloadItem& operator=(const DownloadItem&) = delete;
	};
}
