#include "stdafx.h"
#include <Kortex/NetworkManager.hpp>
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
		m_Stream = std::make_unique<KxFileStream>(GetLocalTempPath(), KxFileStream::Access::Write, disposition, KxFileStream::Share::Read);
		if (m_Stream->IsOk())
		{
			// Notify download manager about start of download
			QueueNotifyEvent(DownloadEvent::EvtStarted);

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
				OnTaskProgress(m_Item);
				NotifyEvent(DownloadEvent::EvtProgress);
			});
		}
	}
	void DownloadExecutor::OnEnd()
	{
		m_EvtHandler.CallAfter([this]()
		{
			// Close the stream now to allow download item or manager to manipulate the file
			m_Stream->Close();

			// Download item relinquished ownership to the executor to us.
			// It will be deleted when we are done executing events dispatching below.
			auto executor = OnTaskEnd(m_Item);

			// Send event to download manager
			if (m_IsCompleted)
			{
				RenameTempFile();
				NotifyEvent(DownloadEvent::EvtCompleted);
			}
			else if (m_Session->IsStopped())
			{
				DeleteTempFile();
				NotifyEvent(DownloadEvent::EvtStopped);
			}
			else
			{
				DeleteTempFile();
				NotifyEvent(DownloadEvent::EvtFailed);
			}
		});
	}

	void DownloadExecutor::QueueNotifyEvent(KxEventTag<DownloadEvent> eventID)
	{
		m_EvtHandler.CallAfter([this, eventID]()
		{
			NotifyEvent(eventID);
		});
	}
	void DownloadExecutor::NotifyEvent(KxEventTag<DownloadEvent> eventID)
	{
		BroadcastProcessor::Get().ProcessEvent(eventID, m_Item);
	}
	void DownloadExecutor::Terminate()
	{
		Stop();
		if (m_Thread)
		{
			m_Thread->Delete();
		}
	}

	bool DownloadExecutor::RenameTempFile()
	{
		return KxFile(GetLocalTempPath()).Rename(m_LocalPath, true);
	}
	bool DownloadExecutor::DeleteTempFile()
	{
		return KxFile(GetLocalTempPath()).RemoveFile();
	}

	DownloadExecutor::DownloadExecutor(DownloadItem& item, const KxURI& uri, const wxString& localPath)
		:m_Item(item), m_DownloadManager(*IDownloadManager::GetInstance()), m_URI(uri), m_LocalPath(localPath)
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

		// Stop LibCURL now. This will cause download thread to terminate itself
		if (m_Session)
		{
			m_Session->Stop();
		}
	}
	bool DownloadExecutor::Pause()
	{
		if (IsRunning() && m_Session->Pause() && m_Thread->Pause() == wxTHREAD_NO_ERROR)
		{
			QueueNotifyEvent(DownloadEvent::EvtPaused);
			return true;
		}
		return false;
	}
	bool DownloadExecutor::Resume()
	{
		if (IsPaused() && m_Session->Resume() && m_Thread->Resume() == wxTHREAD_NO_ERROR)
		{
			QueueNotifyEvent(DownloadEvent::EvtResumed);
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

			m_Session = INetworkManager::GetInstance()->NewCURLSession(m_URI);
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

	std::optional<int64_t> DownloadExecutor::RequestContentLength() const
	{
		auto session = INetworkManager::GetInstance()->NewCURLSession(m_URI);
		if (session)
		{
			int64_t contentLength = -1;
			session->Bind(KxEVT_CURL_DOWNLOAD, [&session, &contentLength](KxCURLEvent& event)
			{
				contentLength = event.GetMajorTotal();
				session->Stop();
			});

			if (contentLength > 0)
			{
				return contentLength;
			}
		}
		return std::nullopt;
	}
	wxString DownloadExecutor::GetLocalPath() const
	{
		return m_LocalPath;
	}
	wxString DownloadExecutor::GetLocalTempPath() const
	{
		// In most cases this path would be 'm_Item.GetLocalTempPath()' but executor could be given different local path than its item
		return m_LocalPath + m_Item.GetTempPathSuffix();
	}
}
