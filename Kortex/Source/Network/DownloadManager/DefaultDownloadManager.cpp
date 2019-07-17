#include "stdafx.h"
#include "DefaultDownloadManager.h"
#include "DownloadExecutor.h"
#include "Workspace.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/NetworkManager.hpp>
#include "UI/KMainWindow.h"
#include <KxFramework/KxFileFinder.h>

namespace Kortex::Application::OName
{
	KortexDefOption(Downloads);
	KortexDefOption(ShowHiddenDownloads);
}

namespace Kortex::DownloadManager
{
	void DefaultDownloadManager::OnInit()
	{
		KxFile(GetDownloadsLocation()).CreateFolder();
	}
	void DefaultDownloadManager::OnExit()
	{
		PauseAllActive();
	}
	void DefaultDownloadManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
	}
	
	KWorkspace* DefaultDownloadManager::CreateWorkspace(KMainWindow* mainWindow)
	{
		return new Workspace(mainWindow);
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
			DownloadItem& item = AddDownload(std::make_unique<DownloadItem>(downloadInfo, fileInfo, modRepository, id));
			return item.Start();
		}
		return false;
	}
}
