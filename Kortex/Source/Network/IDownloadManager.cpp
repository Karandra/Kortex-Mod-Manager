#include "stdafx.h"
#include "IDownloadManager.h"
#include <Kortex/NetworkManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxDrive.h>
#include <KxFramework/KxApp.h>

namespace Kortex
{
	namespace DownloadManager::Internal
	{
		const SimpleManagerInfo TypeInfo("DownloadManager", "DownloadManager.Name");
	}

	wxString IDownloadManager::RenameIncrement(const wxString& name)
	{
		wxRegEx regEx(u8R"((.*)\((\d+)\)\.(.*))", wxRE_ADVANCED|wxRE_ICASE);
		if (regEx.Matches(name))
		{
			int64_t value = 0;
			if (regEx.GetMatch(name, 2).ToLongLong(&value))
			{
				value++;

				wxString newName = name;
				regEx.Replace(&newName, KxString::Format("\\1(%1).\\3", value));
				return newName;
			}
		}
		return KxString::Format("%1 (1).%2", name.BeforeLast('.'), name.AfterLast('.'));
	}
	auto IDownloadManager::CheckDownloadLocation(const wxString& directoryPath, int64_t fileSize) const -> DownloadLocationError
	{
		// Check path and folder existence
		if (directoryPath.IsEmpty())
		{
			return DownloadLocationError::NotSpecified;
		}
		if (!KxFile(directoryPath).IsFolderExist())
		{
			return DownloadLocationError::NotExist;
		}
		
		// Check volume capabilities
		KxDrive drive(directoryPath);

		// Add one MB because it's probably impossible to safely save a file with exact size as free space left on disk
		constexpr int64_t reserveSize = 1024 * 1024 * 1024;
		if (fileSize > 0 && drive.GetFreeSpace() < (fileSize + reserveSize))
		{
			return DownloadLocationError::InsufficientVolumeSpace;
		}

		#if 0
		// Check if the volume supports file streams
		if (!(drive.GetInfo().FileSystemFlags & FILE_NAMED_STREAMS))
		{
			return DownloadLocationError::InsufficientVolumeCapabilities;
		}
		#endif

		return DownloadLocationError::Success;
	}

	IDownloadManager::IDownloadManager()
		:ManagerWithTypeInfo(NetworkModule::GetInstance())
	{
	}
	IDownloadManager::~IDownloadManager()
	{
	}

	void IDownloadManager::PauseAllActive()
	{
		for (const auto& entry: GetDownloads())
		{
			if (entry->IsRunning())
			{
				entry->Pause();
			}
		}
	}

	IDownloadEntry::RefVector IDownloadManager::GetInactiveDownloads(bool installedOnly) const
	{
		IDownloadEntry::RefVector items;
		for (const auto& entry: GetDownloads())
		{
			if (!entry->IsRunning())
			{
				if (installedOnly && !entry->IsInstalled())
				{
					continue;
				}
				items.push_back(entry.get());
			}
		}
		return items;
	}
	IDownloadEntry* IDownloadManager::FindDownloadByFileName(const wxString& name, const IDownloadEntry* except) const
	{
		for (const auto& entry: GetDownloads())
		{
			if (entry.get() != except && KxComparator::IsEqual(name, entry->GetFileInfo().Name))
			{
				return entry.get();
			}
		}
		return nullptr;
	}
	
	void IDownloadManager::AutoRenameIncrement(IDownloadEntry& entry) const
	{
		while (FindDownloadByFileName(entry.GetFileInfo().Name, &entry))
		{
			entry.GetFileInfo().Name = RenameIncrement(entry.GetFileInfo().Name);
		}
	}
}

namespace Kortex
{
	bool IDownloadManagerNXM::CheckCmdLineArgs(const wxCmdLineParser& args, wxString& link)
	{
		return args.Found("NXM", &link);
	}
}
