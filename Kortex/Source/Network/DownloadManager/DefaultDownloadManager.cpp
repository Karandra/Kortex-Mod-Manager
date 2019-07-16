#include "stdafx.h"
#include "DefaultDownloadManager.h"
#include "DownloadExecutor.h"
#include "DisplayModel.h"
#include "Workspace.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/Notification.hpp>
#include <Kortex/GameInstance.hpp>
#include "UI/KMainWindow.h"
#include "Utility/KAux.h"
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxDataView.h>

namespace
{
	using namespace Kortex;
	using namespace Kortex::DownloadManager;

	DisplayModel* GetViewAndItem(const DownloadItem& entry, KxDataViewItem& item)
	{
		Workspace* workspace = Workspace::GetInstance();
		if (workspace)
		{
			DisplayModel* view = workspace->GetModelView();
			if (view)
			{
				item = view->FindItem(entry);
				return view;
			}
		}

		item = KxDataViewItem();
		return nullptr;
	}
}

namespace Kortex::Application::OName
{
	KortexDefOption(Downloads);
	KortexDefOption(ShowHiddenDownloads);
}

namespace Kortex::DownloadManager
{
	KWorkspace* DefaultDownloadManager::CreateWorkspace(KMainWindow* mainWindow)
	{
		return new Workspace(mainWindow);
	}

	void DefaultDownloadManager::OnDownloadEvent(const DownloadItem& item, ItemEvent eventType)
	{
		bool allowSave = true;
		bool allowUpdate = true;

		switch (eventType)
		{
			case ItemEvent::Added:
			case ItemEvent::Changed:
			case ItemEvent::Stopped:
			case ItemEvent::Paused:
			case ItemEvent::Resumed:
			{
				break;
			}
			case ItemEvent::Started:
			{
				INotificationCenter::Notify(KTr("DownloadManager.Notification.DownloadStarted"),
											KTrf("DownloadManager.Notification.DownloadStartedEx", item.GetFileInfo().Name),
											KxICON_INFORMATION
				);
				break;
			}
			case ItemEvent::Completed:
			{
				INotificationCenter::Notify(KTr("DownloadManager.Notification.DownloadCompleted"),
											KTrf("DownloadManager.Notification.DownloadCompletedEx", item.GetFileInfo().Name),
											KxICON_INFORMATION
				);
				break;
			}
			case ItemEvent::Failed:
			{
				INotificationCenter::Notify(KTr("DownloadManager.Notification.DownloadFailed"),
											KTrf("DownloadManager.Notification.DownloadFailedEx", item.GetFileInfo().Name),
											KxICON_WARNING
				);
				break;
			}
			case ItemEvent::Removed:
			case ItemEvent::Progress:
			{
				allowSave = false;
				break;
			}
			default:
			{
				allowSave = false;
				allowUpdate = false;
			}
		};

		if (m_IsReady)
		{
			if (allowUpdate)
			{
				KxDataViewItem viewItem;
				DisplayModel* view = GetViewAndItem(item, viewItem);
				
				if (viewItem.IsOK())
				{
					view->ItemChanged(viewItem);
				}
				else
				{
					view->RefreshItems();
				}
			}
			if (allowSave)
			{
				item.Save();
			}
		}
	}

	void DefaultDownloadManager::OnInit()
	{
		m_IsReady = true;
		KxFile(GetDownloadsLocation()).CreateFolder();
	}
	void DefaultDownloadManager::OnExit()
	{
		PauseAllActive();
		m_IsReady = false;
	}
	void DefaultDownloadManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
	}

	void DefaultDownloadManager::LoadDownloads()
	{
		PauseAllActive();
		m_Downloads.clear();

		bool showHidden = ShouldShowHiddenDownloads();

		KxFileFinder finder(GetDownloadsLocation(), "*");
		for (KxFileItem fileItem = finder.FindNext(); fileItem.IsOK(); fileItem = finder.FindNext())
		{
			if (fileItem.IsNormalItem() && fileItem.IsFile())
			{
				DownloadItem& download = *m_Downloads.emplace_back(std::make_unique<DownloadItem>());

				// Try to load from download xml or load as much as possible from download itself
				if (!download.Load(fileItem))
				{
					download.LoadDefault(fileItem);
					download.Save();
				}
			}
		}
	}
	void DefaultDownloadManager::SaveDownloads()
	{
		for (const auto& entry: m_Downloads)
		{
			if (!entry->IsRunning())
			{
				entry->Save();
			}
		}
	}

	bool DefaultDownloadManager::ShouldShowHiddenDownloads() const
	{
		using namespace Application;
		return GetAInstanceOption(OName::ShowHiddenDownloads).GetValueBool();
	}
	void DefaultDownloadManager::ShowHiddenDownloads(bool show)
	{
		using namespace Application;
		GetAInstanceOption(OName::ShowHiddenDownloads).SetValue(show);
	}

	wxString DefaultDownloadManager::GetDownloadsLocation() const
	{
		using namespace Application;

		auto option = GetAInstanceOption(OName::Downloads);
		return option.GetAttribute(OName::Location);
	}
	void DefaultDownloadManager::SetDownloadsLocation(const wxString& location)
	{
		using namespace Application;

		GetAInstanceOption(OName::Downloads).SetAttribute(OName::Location, location);
		KxFile(location).CreateFolder();
	}
	auto DefaultDownloadManager::OnAccessDownloadLocation(int64_t fileSize) const -> LocationStatus
	{
		const LocationStatus status = CheckDownloadLocation(GetDownloadsLocation(), fileSize);
		switch (status)
		{
			case LocationStatus::NotExist:
			case LocationStatus::NotSpecified:
			{
				INotificationCenter::Notify(KTr("DownloadManager.DownloadLocation"),
											KTr("DownloadManager.DownloadLocationInvalid"),
											KxICON_ERROR
				);
				break;
			}
			case LocationStatus::InsufficientVolumeSpace:
			{
				INotificationCenter::Notify(KTr("DownloadManager.DownloadLocation"),
											KTr("DownloadManager.DownloadLocationInsufficientSpace"),
											KxICON_ERROR
				);
				break;
			}
			case LocationStatus::InsufficientVolumeCapabilities:
			{
				INotificationCenter::Notify(KTr("DownloadManager.DownloadLocation"),
											KTr("DownloadManager.DownloadLocationInsufficientCapabilities"),
											KxICON_ERROR
				);
				break;
			}
		};
		return status;
	}

	std::unique_ptr<IDownloadExecutor> DefaultDownloadManager::NewDownloadExecutor(DownloadItem& item,
																				   const KxURI& url,
																				   const wxString& localPath
	)
	{
		return std::make_unique<DownloadExecutor>(item, url, localPath);
	}
	bool DefaultDownloadManager::QueueDownload(ModNetworkRepository& modRepository,
											   const ModDownloadReply& downloadInfo,
											   const ModFileReply& fileInfo,
											   const GameID& id
	)
	{
		if (OnAccessDownloadLocation(fileInfo.Size) != LocationStatus::Success)
		{
			return false;
		}

		if (downloadInfo.IsOK() && fileInfo.IsOK())
		{
			DownloadItem& item = *m_Downloads.emplace_back(std::make_unique<DownloadItem>(downloadInfo, fileInfo, modRepository, id));
			AutoRenameIncrement(item);
			OnDownloadEvent(item, ItemEvent::Added);

			return item.Start();
		}
		return false;
	}
}
