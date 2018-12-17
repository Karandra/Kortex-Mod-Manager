#include "stdafx.h"
#include "IDownloadManager.h"
#include <Kortex/NetworkManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxFile.h>
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

	IDownloadManager::IDownloadManager()
		:ManagerWithTypeInfo(NetworkModule::GetInstance())
	{
	}
	IDownloadManager::~IDownloadManager()
	{
		PauseAllActive();
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

	IDownloadEntry::RefVector IDownloadManager::GetNotRunningDownloads(bool installedOnly) const
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
			if (entry.get() != except && KxComparator::IsEqual(name, entry->GetFileInfo().GetName()))
			{
				return entry.get();
			}
		}
		return nullptr;
	}
	
	void IDownloadManager::AutoRenameIncrement(IDownloadEntry& entry) const
	{
		while (FindDownloadByFileName(entry.GetFileInfo().GetName(), &entry))
		{
			entry.GetFileInfo().SetName(RenameIncrement(entry.GetFileInfo().GetName()));
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
