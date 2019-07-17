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
	void DefaultDownloadManager::OnInit()
	{
		KxFile(GetDownloadsLocation()).CreateFolder();

		IDownloadManager::OnInit();
	}
	void DefaultDownloadManager::OnExit()
	{
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
			return item.Start();
		}
		return false;
	}
}
