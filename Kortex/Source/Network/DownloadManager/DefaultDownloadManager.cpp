#include "stdafx.h"
#include "DefaultDownloadManager.h"
#include "DownloadExecutor.h"
#include "Workspace.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/NetworkManager.hpp>
#include "UI/KMainWindow.h"
#include <KxFramework/KxFileFinder.h>


namespace Kortex::DownloadManager
{
	DownloadItem* DefaultDownloadManager::CheckQueue()
	{
		if (HasConcurrentDownloadsLimit() && !m_Queue.empty())
		{
			if (GetActiveDownloadsCount() < GetMaxConcurrentDownloads())
			{
				DownloadItem* item = m_Queue.back();
				m_Queue.pop_back();
				return item;
			}
		}
		return nullptr;
	}
	bool DefaultDownloadManager::TryStartDownload()
	{
		if (DownloadItem* item = CheckQueue())
		{
			return item->Start();
		}
		return false;
	}

	void DefaultDownloadManager::OnDownloadStarted(DownloadEvent& event)
	{
		TryStartDownload();
	}
	void DefaultDownloadManager::OnDownloadCompleted(DownloadEvent& event)
	{
		TryStartDownload();
	}
	void DefaultDownloadManager::OnDownloadPaused(DownloadEvent& event)
	{
		TryStartDownload();
	}
	void DefaultDownloadManager::OnDownloadResumed(DownloadEvent& event)
	{
		TryStartDownload();
	}
	void DefaultDownloadManager::OnDownloadRemoved(DownloadEvent& event)
	{
		if (HasConcurrentDownloadsLimit())
		{
			for (auto it = m_Queue.begin(); it != m_Queue.end(); ++it)
			{
				if (*it == &event.GetDownload())
				{
					m_Queue.erase(it);
					return;
				}
			}
		}
	}

	void DefaultDownloadManager::OnInit()
	{
		IDownloadManager::OnInit();
		m_Queue.reserve(HasConcurrentDownloadsLimit() ? GetMaxConcurrentDownloads() * 2 : 0);

		IEvent::Bind(DownloadEvent::EvtStarted, &DefaultDownloadManager::OnDownloadStarted, this);
		IEvent::Bind(DownloadEvent::EvtCompleted, &DefaultDownloadManager::OnDownloadCompleted, this);
		IEvent::Bind(DownloadEvent::EvtPaused, &DefaultDownloadManager::OnDownloadPaused, this);
		IEvent::Bind(DownloadEvent::EvtResumed, &DefaultDownloadManager::OnDownloadResumed, this);
		IEvent::Bind(DownloadEvent::EvtRemoved, &DefaultDownloadManager::OnDownloadRemoved, this);
	}
	void DefaultDownloadManager::OnExit()
	{
		IEvent::Unbind(DownloadEvent::EvtStarted, &DefaultDownloadManager::OnDownloadStarted, this);
		IEvent::Unbind(DownloadEvent::EvtCompleted, &DefaultDownloadManager::OnDownloadCompleted, this);
		IEvent::Unbind(DownloadEvent::EvtPaused, &DefaultDownloadManager::OnDownloadPaused, this);
		IEvent::Unbind(DownloadEvent::EvtResumed, &DefaultDownloadManager::OnDownloadResumed, this);
		IEvent::Unbind(DownloadEvent::EvtRemoved, &DefaultDownloadManager::OnDownloadRemoved, this);

		IDownloadManager::OnExit();
	}
	void DefaultDownloadManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		IDownloadManager::OnLoadInstance(instance, managerNode);
	}
	
	KWorkspace* DefaultDownloadManager::CreateWorkspace(KMainWindow* mainWindow)
	{
		return new Workspace(mainWindow);
	}

	std::unique_ptr<IDownloadExecutor> DefaultDownloadManager::NewDownloadExecutor(DownloadItem& item,
																				   const KxURI& uri,
																				   const wxString& localPath
	)
	{
		return std::make_unique<DownloadExecutor>(item, uri, localPath);
	}
	bool DefaultDownloadManager::QueueDownload(ModNetworkRepository& modRepository,
											   const ModDownloadReply& downloadInfo,
											   const ModFileReply& fileInfo,
											   const GameID& id
	)
	{
		if (fileInfo.HasSize() && OnAccessDownloadLocation(fileInfo.Size) != LocationStatus::Success)
		{
			return false;
		}

		if (downloadInfo.IsOK() && fileInfo.IsOK())
		{
			DownloadItem& item = AddDownload(std::make_unique<DownloadItem>(downloadInfo, fileInfo, modRepository, id));
			if (HasConcurrentDownloadsLimit())
			{
				m_Queue.insert(m_Queue.begin(), &item);
				return TryStartDownload();
			}
			else
			{
				return item.Start();
			}
		}
		return false;
	}
}
