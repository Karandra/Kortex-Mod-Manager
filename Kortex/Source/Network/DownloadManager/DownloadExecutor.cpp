#include "stdafx.h"
#include <Kortex/NetworkManager.hpp>
#include <Kortex/Events.hpp>
#include "DownloadExecutor.h"
#include "Utility/DateTime.h"
#include <chrono>

namespace
{
	wxTimeSpan GetClockTime()
	{
		using namespace std::chrono;
		return wxTimeSpan::Milliseconds(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
	};
}

namespace Kortex::DownloadManager
{
	wxThread::ExitCode DownloadExecutor::Entry()
	{
		OnStart();
		OnEnd();

		return 0;
	}
	void DownloadExecutor::OnStart()
	{
		KxFileStream::Disposition disposition = m_StartDownloadAt > 0 ? KxFileStream::Disposition::OpenExisting : KxFileStream::Disposition::CreateAlways;
		m_Stream = std::make_unique<KxFileStream>(m_LocalPath, KxFileStream::Access::Write, disposition, KxFileStream::Share::Read);
		if (m_Stream->IsOk())
		{
			// Notify download manager about start of download
			QueueNotifyEvent(ItemEvent::Started);

			// Begin download, blocking operation.
			m_TimeStamp = GetClockTime();
			KxCURLStreamReply reply(*m_Stream, m_StartDownloadAt);
			m_Session->Download(reply);

			// Notify result
			if (!m_Session->IsStopped() && reply.IsOK() && m_DownloadedSize == m_Stream->GetLength())
			{
				m_IsCompleted = true;
				return;
			}
		}
		m_IsFailed = true;
	}
	void DownloadExecutor::OnDownload(KxCURLEvent& event)
	{
		if (m_Thread->TestDestroy())
		{
			m_Session->Stop();
		}

		// Update downloaded size and speed
		m_DownloadedSize = event.GetMajorProcessed();
		m_Speed = event.GetSpeed();
		
		// Update file size. Nexus reports file size in KB, so initial info maybe
		// incorrect, see 'NexusModNetwork::ReadFileInfo<T>' function for details.
		// Other sources can return correct sizes, but it's better to request it here.
		if (int64_t totalSize = event.GetMajorTotal(); totalSize > 0)
		{
			KxCURLStreamReply& reply = static_cast<KxCURLStreamReply&>(event.GetReply());
			m_TotalSize = totalSize + reply.GetResumeFromPosition();
		}
		else
		{
			// We don't know total size, so use downloaded size
			m_TotalSize = event.GetMajorProcessed();
		}

		// Queue update
		wxTimeSpan time = GetClockTime();
		if (time - m_TimeStamp >= wxTimeSpan::Milliseconds(250))
		{
			m_TimeStamp = time;
			m_EvtHandler.CallAfter([this]()
			{
				m_Item.OnUpdateProgress();
				NotifyEvent(ItemEvent::Progress);
			});
		}
	}
	void DownloadExecutor::OnEnd()
	{
		m_EvtHandler.CallAfter([this]()
		{
			// Download item relinquished ownership to the executor to us.
			// It will be deleted when we are done executing events dispatching below.
			auto executor = m_Item.OnExecutorDone();

			// Send event to download manager
			if (m_IsCompleted)
			{
				NotifyEvent(ItemEvent::Completed);
			}
			else if (m_Session->IsStopped())
			{
				NotifyEvent(ItemEvent::Stopped);
			}
			else
			{
				NotifyEvent(ItemEvent::Failed);
			}
		});
	}

	void DownloadExecutor::QueueNotifyEvent(ItemEvent eventType)
	{
		m_EvtHandler.CallAfter([this, eventType]()
		{
			NotifyEvent(eventType);
		});
	}
	void DownloadExecutor::NotifyEvent(ItemEvent eventType)
	{
		m_DownloadManager.OnDownloadEvent(m_Item, eventType);
	}
	void DownloadExecutor::Terminate()
	{
		Stop();
		if (m_Thread)
		{
			m_Thread->Delete();
		}
	}

	DownloadExecutor::DownloadExecutor(DownloadItem& item, const KxURI& url, const wxString& localPath)
		:m_Item(item), m_DownloadManager(*IDownloadManager::GetInstance()), m_URL(url), m_LocalPath(localPath)
	{
	}
	DownloadExecutor::~DownloadExecutor()
	{
		Terminate();
	}

	void DownloadExecutor::Stop()
	{
		// If download is paused resume it first
		if (IsPaused())
		{
			m_Session->Resume();
			m_Thread->Resume();
		}

		// Stop CURL now. This will cause download thread to terminate itself
		if (m_Session)
		{
			m_Session->Stop();
		}
	}
	bool DownloadExecutor::Pause()
	{
		if (IsRunning() && m_Session->Pause() && m_Thread->Pause() == wxTHREAD_NO_ERROR)
		{
			QueueNotifyEvent(ItemEvent::Paused);
			return true;
		}
		return false;
	}
	bool DownloadExecutor::Resume()
	{
		if (IsPaused() && m_Session->Resume() && m_Thread->Resume() == wxTHREAD_NO_ERROR)
		{
			QueueNotifyEvent(ItemEvent::Resumed);
			return true;
		}
		return false;
	}
	bool DownloadExecutor::Start(int64_t startAt)
	{
		m_StartDownloadAt = startAt;
		if (CreateThread(wxTHREAD_JOINABLE) == wxTHREAD_NO_ERROR)
		{
			m_StartDate = Utility::DateTime::Now();

			m_Session = INetworkManager::GetInstance()->NewCURLSession(m_URL);
			m_Session->Bind(KxEVT_CURL_DOWNLOAD, &DownloadExecutor::OnDownload, this);

			m_Thread = GetThread();
			return m_Thread->Run() == wxTHREAD_NO_ERROR;
		}
		else
		{
			m_IsFailed = true;
			OnEnd();

			return false;
		}
	}

	int64_t DownloadExecutor::RequestContentLength() const
	{
		int64_t contentLength = -1;

		auto session = INetworkManager::GetInstance()->NewCURLSession(m_URL);
		session->Bind(KxEVT_CURL_DOWNLOAD, [&session, &contentLength](KxCURLEvent& event)
		{
			contentLength = event.GetMajorTotal();
			session->Stop();
		});
		return contentLength;
	}
}
