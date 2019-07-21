#include "stdafx.h"
#include "DefaultDownloadManager.h"
#include "DownloadExecutor.h"
#include "Workspace.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/NetworkManager.hpp>
#include "UI/KMainWindow.h"
#include <KxFramework/KxFileFinder.h>
#include <Kx/Async/DelayedCall.h>

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

	void DefaultDownloadManager::OnNeedToStartDownload(DownloadEvent& event)
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

		m_BroadcastReciever.Bind(DownloadEvent::EvtStarted, &DefaultDownloadManager::OnNeedToStartDownload, this);
		m_BroadcastReciever.Bind(DownloadEvent::EvtCompleted, &DefaultDownloadManager::OnNeedToStartDownload, this);
		m_BroadcastReciever.Bind(DownloadEvent::EvtPaused, &DefaultDownloadManager::OnNeedToStartDownload, this);
		m_BroadcastReciever.Bind(DownloadEvent::EvtResumed, &DefaultDownloadManager::OnNeedToStartDownload, this);
		m_BroadcastReciever.Bind(DownloadEvent::EvtConcurrentDownloadsCountChanged, &DefaultDownloadManager::OnNeedToStartDownload, this);

		m_BroadcastReciever.Bind(DownloadEvent::EvtRemoved, &DefaultDownloadManager::OnDownloadRemoved, this);
	}
	void DefaultDownloadManager::OnExit()
	{
		m_BroadcastReciever.UnbindAll();
		IDownloadManager::OnExit();
	}
	void DefaultDownloadManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		IDownloadManager::OnLoadInstance(instance, managerNode);
	}
	
	KWorkspace* DefaultDownloadManager::CreateWorkspace(KMainWindow* mainWindow)
	{
		// Create workspace
		Workspace* workspace = new Workspace(mainWindow);

		// If we're the primary instance and we were launched with link as command line
		// argument we need to manually queue it since IApplication instance merely checks
		// its existence and otherwise does nothing with it.
		Kx::Async::DelayedCall([]()
		{
			if (auto link = IApplication::GetInstance()->GetLinkFromCommandLine())
			{
				IDownloadManager::GetInstance()->QueueUnknownDownload(*link);
			}
		}, wxTimeSpan::Seconds(2));

		// Return the workspace
		return workspace;
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
