#pragma once
#include "stdafx.h"
#include "Network/IDownloadEntry.h"
#include "Network/IModNetwork.h"
#include "Network/IModNetworkRepository.h"
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxFileStream.h>
class KQuickThread;
class KxFileItem;

namespace Kortex
{
	class IGameMod;
	class IGameInstance;
}

namespace Kortex::DownloadManager
{
	class DefaultDownloadEntry: public IDownloadEntry
	{
		private:
			ModFileReply m_FileInfo;
			ModDownloadReply m_DownloadInfo;

			wxEvtHandler m_EvtHandler;
			KQuickThread* m_Thread = nullptr;
			std::unique_ptr<KxFileStream> m_Stream;
			std::unique_ptr<KxCURLSession> m_Session;

			const IGameInstance* m_TargetGame = nullptr;
			IModNetwork* m_ModSource = nullptr;

			wxDateTime m_Date;
			int64_t m_DownloadedSize = 0;
			int64_t m_Speed = 0;
			int64_t m_TimeStamp = 0;

			bool m_IsPaused = false;
			bool m_IsHidden = false;
			bool m_IsFailed = false;

		private:
			void Create();
			void CleanupDownload();
			bool RequestNewLink();

			void OnDownload(KxCURLEvent& event);
			void OnThreadExit(wxNotifyEvent& event);
			void DoRun(int64_t resumePos = 0);

		public:
			DefaultDownloadEntry();
			DefaultDownloadEntry(const ModDownloadReply& downloadInfo,
								 const ModFileReply& fileInfo,
								 IModNetworkRepository& modRepository,
								 const GameID& id = {});
			virtual ~DefaultDownloadEntry();

		public:
			bool IsOK() const override
			{
				return m_ModSource && m_TargetGame && m_FileInfo.IsOK() && m_DownloadInfo.IsOK();
			}
			wxString GetFullPath() const override;
			wxString GetMetaFilePath() const override;
			
			const IGameInstance* GetTargetGame() const override
			{
				return m_TargetGame;
			}
			void SetTargetGame(const IGameInstance* instance) override
			{
				m_TargetGame = instance;
			}

			const IGameMod* GetMod() const override;
			bool IsInstalled() const override;

			IModNetwork* GetModNetwork() const override
			{
				return m_ModSource;
			}
			void SetModNetwork(IModNetworkRepository& modRepository) override
			{
				m_ModSource = modRepository.QueryInterface<IModNetwork>();
			}
			
			wxDateTime GetDate() const override
			{
				return m_Date;
			}
			void SetDate(const wxDateTime& date) override
			{
				m_Date = date;
			}
			
			int64_t GetDownloadedSize() const override
			{
				return m_DownloadedSize;
			}
			void SetDownloadedSize(int64_t size) override
			{
				m_DownloadedSize = size;
			}

			int64_t GetSpeed() const override
			{
				return m_Speed;
			}
			bool IsCompleted() const override
			{
				return !IsFailed() && m_FileInfo.Size == m_DownloadedSize;
			}
			bool CanRestart() const override
			{
				return !IsRunning() && m_ModSource && m_FileInfo.IsOK();
			}

			const ModFileReply& GetFileInfo() const override
			{
				return m_FileInfo;
			}
			ModFileReply& GetFileInfo() override
			{
				return m_FileInfo;
			}

			const ModDownloadReply& GetDownloadInfo() const override
			{
				return m_DownloadInfo;
			}
			ModDownloadReply& GetDownloadInfo() override
			{
				return m_DownloadInfo;
			}

			bool IsRunning() const override
			{
				return m_Thread != nullptr;
			}
			bool IsPaused() const override
			{
				return m_IsPaused;
			}
			bool IsFailed() const override
			{
				return m_IsFailed;
			}
			
			void SetPaused(bool value) override
			{
				m_IsPaused = value;
			}
			void SetFailed(bool value) override
			{
				m_IsFailed = value;
			}

			bool IsHidden() const override
			{
				return m_IsHidden && !IsRunning();
			}
			void SetHidden(bool value) override
			{
				m_IsHidden = value;
			}

			void Stop() override;
			void Pause() override;
			void Resume() override;
			void Run(int64_t resumeFrom = 0) override;
			bool Restart() override;

			// Restores download info depending of this download modNetwork and its filename
			// which is set in 'DefaultDownloadEntry::DeSerializeDefault' if something goes wrong.
			// Restoration is performed by analyzing file name to get file id and mod id
			// and querying rest of the information form internet.
			bool RepairBrokedDownload() override;
			bool QueryInfo() override;

		public:
			bool Serialize(wxOutputStream& stream) const override;
			bool DeSerialize(wxInputStream& stream) override;
			void LoadDefault(const KxFileItem& fileItem) override;
	};
}
