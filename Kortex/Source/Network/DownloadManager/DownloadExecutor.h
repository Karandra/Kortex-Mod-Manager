#pragma once
#include "stdafx.h"
#include "Network/IModNetwork.h"
#include "Network/ModNetworkRepository.h"
#include "DownloadEvent.h"
#include "IDownloadExecutor.h"
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxFileStream.h>

namespace Kortex
{
	class DownloadItem;
	class IDownloadManager;
}
namespace Kortex::DownloadManager
{
	class DownloadExecutor: public IDownloadExecutor, public wxThreadHelper
	{
		private:
			wxEvtHandler m_EvtHandler;
			IDownloadManager& m_DownloadManager;
			DownloadItem& m_Item;

			wxString m_LocalPath;
			KxURI m_URI;

			wxThread* m_Thread = nullptr;
			std::unique_ptr<KxFileStream> m_Stream;
			std::unique_ptr<KxCURLSession> m_Session;
			wxTimeSpan m_TimeStamp;
			int64_t m_StartDownloadAt = 0;
			bool m_IsCompleted = false;

			int64_t m_DownloadedSize = 0;
			int64_t m_TotalSize = 0;
			int64_t m_Speed = 0;
			bool m_IsFailed = false;
			wxDateTime m_StartDate;

		private:
			wxThread::ExitCode Entry() override;
			void OnStart();
			void OnDownload(KxCURLEvent& event);
			void OnEnd();

			void NotifyEvent(wxEventTypeTag<DownloadEvent> eventType);
			void QueueNotifyEvent(wxEventTypeTag<DownloadEvent> eventType);
			void Terminate();

		public:
			DownloadExecutor(DownloadItem& item, const KxURI& uri, const wxString& localPath);
			~DownloadExecutor();

		public:
			bool IsRunning() const override
			{
				return m_Thread && m_Thread->IsRunning();
			}
			bool IsPaused() const override
			{
				return m_Thread && m_Thread->IsPaused();
			}
			bool IsFailed() const override
			{
				return m_IsFailed;
			}
			bool IsCompleted() const override
			{
				return !IsFailed() && m_IsCompleted;
			}

			void Stop() override;
			bool Pause() override;
			bool Resume() override;
			bool Start(int64_t startAt = 0) override;

			int64_t GetSpeed() const override
			{
				return m_Speed;
			}
			int64_t GetTotalSize() const override
			{
				return m_TotalSize;
			}
			int64_t GetDownloadedSize() const override
			{
				return m_DownloadedSize;
			}
			int64_t RequestContentLength() const override;
			wxDateTime GetStartDate() const override
			{
				return m_StartDate;
			}
	};
}
