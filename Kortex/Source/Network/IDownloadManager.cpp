#include "stdafx.h"
#include "IDownloadManager.h"
#include <Kortex/NetworkManager.hpp>
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxDrive.h>
#include <KxFramework/KxRegistry.h>
#include <KxFramework/KxUtility.h>

namespace
{
	template<class T> static auto GetDownloadIterator(T&& items, const Kortex::DownloadItem& item)
	{
		return std::find_if(items.begin(), items.end(), [&item](const auto& v)
		{
			return v.get() == &item;
		});
	}
}

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
	void IDownloadManager::ConfigureCommandLine(wxCmdLineParser& parser)
	{
		parser.AddOption(wxS("DownloadLink"), wxEmptyString, "Download link");
	}
	KxURI IDownloadManager::GetLinkFromCommandLine(const wxCmdLineParser& parser)
	{
		wxString link;
		if (parser.Found(wxS("DownloadLink"), &link) && !link.IsEmpty())
		{
			return link;
		}
		return {};
	}

	bool IDownloadManager::IsAssociatedWithLink(const wxString& type)
	{
		wxAny path = KxRegistry::GetValue(KxREG_HKEY_CLASSES_ROOT, wxS("NXM\\shell\\open\\command"), "", KxREG_VALUE_SZ, KxREG_NODE_SYS, true);
		return IApplication::GetInstance()->GetExecutablePath() == path.As<wxString>().AfterFirst('"').BeforeFirst('"');
	}
	void IDownloadManager::AssociateWithLink(const wxString& type)
	{
		wxString appPath = IApplication::GetInstance()->GetExecutablePath();
		
		wxString linkType = type.Upper();
		linkType.StartsWith(wxS("."), &linkType);

		auto SetValue = [&appPath, &linkType](const wxString& subKey, bool protocol = false)
		{
			if (protocol)
			{
				KxRegistry::SetValue(KxREG_HKEY_CLASSES_ROOT, subKey, "", KxString::Format("URL:%1 Protocol", linkType), KxREG_VALUE_SZ);
				KxRegistry::SetValue(KxREG_HKEY_CLASSES_ROOT, subKey, "URL Protocol", KxString::Format("URL:%1 Protocol", linkType), KxREG_VALUE_SZ);
			}
			else
			{
				KxRegistry::SetValue(KxREG_HKEY_CLASSES_ROOT, subKey, "", IApplication::GetInstance()->GetShortName() + " Download Link", KxREG_VALUE_SZ);
			}
			KxRegistry::SetValue(KxREG_HKEY_CLASSES_ROOT, subKey + "\\DefaultIcon", "", appPath, KxREG_VALUE_SZ);
			KxRegistry::SetValue(KxREG_HKEY_CLASSES_ROOT, subKey + "\\shell\\open\\command", "", KxString::Format("\"%1\" -DownloadLink \"%2\"", appPath, "%1"), KxREG_VALUE_SZ);
		};

		SetValue(linkType, true);
		SetValue(linkType + wxS("_File_Type"));
	}

	auto IDownloadManager::CheckDownloadLocation(const wxString& directoryPath, int64_t fileSize) const -> LocationStatus
	{
		// Check path and folder existence
		if (directoryPath.IsEmpty())
		{
			return LocationStatus::NotSpecified;
		}
		if (!KxFile(directoryPath).IsFolderExist())
		{
			return LocationStatus::NotExist;
		}
		
		// Check volume capabilities
		KxDrive drive(directoryPath);

		// Add one megabyte because it's probably impossible to safely save a file
		// with exactly the same size as the free space left on the disk.
		constexpr int64_t reserveSize = 1024 * 1024 * 1024;
		if (fileSize > 0 && drive.GetFreeSpace() < (fileSize + reserveSize))
		{
			return LocationStatus::InsufficientVolumeSpace;
		}

		#if 0
		// Check if the volume supports file streams
		if (!(drive.GetInfo().FileSystemFlags & FILE_NAMED_STREAMS))
		{
			return LocationStatus::InsufficientVolumeCapabilities;
		}
		#endif

		return LocationStatus::Success;
	}

	IDownloadManager::IDownloadManager()
		:ManagerWithTypeInfo(NetworkModule::GetInstance())
	{
	}
	IDownloadManager::~IDownloadManager()
	{
	}

	DownloadItem::RefVector IDownloadManager::GetDownloads() const
	{
		return KxUtility::ConvertVector<DownloadItem*>(m_Downloads, [](auto& item)
		{
			return item.get();
		});
	}

	void IDownloadManager::PauseAllActive()
	{
		for (const auto& entry: m_Downloads)
		{
			if (entry->IsRunning())
			{
				entry->Pause();
			}
		}
	}
	DownloadItem::RefVector IDownloadManager::GetInactiveDownloads(bool installedOnly) const
	{
		DownloadItem::RefVector items;
		for (const auto& entry: m_Downloads)
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
	DownloadItem* IDownloadManager::FindDownloadByFileName(const wxString& name, const DownloadItem* except) const
	{
		for (const auto& item: m_Downloads)
		{
			if (item.get() != except && KxComparator::IsEqual(name, item->GetName()))
			{
				return item.get();
			}
		}
		return nullptr;
	}
	void IDownloadManager::AutoRenameIncrement(DownloadItem& item) const
	{
		while (FindDownloadByFileName(item.GetName(), &item))
		{
			item.m_FileInfo.Name = RenameIncrement(item.GetName());
		}
	}

	DownloadItem& IDownloadManager::AddDownload(std::unique_ptr<DownloadItem> download)
	{
		DownloadItem& ref = *m_Downloads.emplace_back(std::move(download));
		AutoRenameIncrement(ref);

		IEvent::MakeSend<DownloadEvent>(DownloadEvent::EvtAdded, ref);

		return ref;
	}
	bool IDownloadManager::RemoveDownload(DownloadItem& download)
	{
		if (!download.IsRunning())
		{
			auto it = GetDownloadIterator(m_Downloads, download);
			if (it != m_Downloads.end())
			{
				// Remove download file
				KxFile(download.GetFullPath()).RemoveFile(true);

				// Erase the item
				auto temp = std::move(*it);
				m_Downloads.erase(it);

				IEvent::MakeSend<DownloadEvent>(DownloadEvent::EvtRemoved, *temp);
				return true;
			}
		}
		return false;
	}

	bool IDownloadManager::TryQueueDownloadLink(const KxURI& link)
	{
		for (ModNetworkRepository* repository: INetworkManager::GetInstance()->GetModRepositories())
		{
			if (repository->QueueDownload(link))
			{
				return true;
			}
		}
		return false;
	}
}
