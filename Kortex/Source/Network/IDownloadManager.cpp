#include "stdafx.h"
#include "IDownloadManager.h"
#include <Kortex/NetworkManager.hpp>
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxDrive.h>
#include <KxFramework/KxRegistry.h>
#include <KxFramework/KxFileFinder.h>
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

namespace Kortex::Application
{
	namespace OName
	{
		KortexDefOption(Downloads);
		KortexDefOption(ShowHidden);
		KortexDefOption(MaxConcurrentDownloads);
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

	void IDownloadManager::OnInit()
	{
		using namespace Application;
		m_Location = GetAInstanceOption(OName::Downloads).GetAttribute(OName::Location);
		m_ShowHiddenDownloads = GetAInstanceOption(OName::ShowHidden).GetValueBool(m_ShowHiddenDownloads);
		m_MaxConcurrentDownloads = GetAInstanceOption(OName::MaxConcurrentDownloads).GetValueInt(m_MaxConcurrentDownloads);

		KxFile(GetDownloadsLocation()).CreateFolder();
	}
	void IDownloadManager::OnExit()
	{
		using namespace Application;
		GetAInstanceOption(OName::Downloads).SetAttribute(OName::Location, m_Location);
		GetAInstanceOption(OName::ShowHidden).SetValue(m_ShowHiddenDownloads);
		GetAInstanceOption(OName::MaxConcurrentDownloads).SetValue(m_MaxConcurrentDownloads);

		PauseAllActive();
		SaveDownloads();
	}
	void IDownloadManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
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

		// Check if the volume supports file streams
		if (!(drive.GetInfo().FileSystemFlags & FILE_NAMED_STREAMS))
		{
			return LocationStatus::InsufficientVolumeCapabilities;
		}

		return LocationStatus::Success;
	}
	auto IDownloadManager::OnAccessDownloadLocation(int64_t fileSize) const -> LocationStatus
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
	DownloadItem::RefVector IDownloadManager::GetInactiveDownloads(bool installedOnly) const
	{
		DownloadItem::RefVector refItems;
		for (const auto& item: m_Downloads)
		{
			if (!item->IsRunning())
			{
				if (installedOnly && !item->IsInstalled())
				{
					continue;
				}
				refItems.push_back(item.get());
			}
		}
		return refItems;
	}
	size_t IDownloadManager::GetActiveDownloadsCount() const
	{
		size_t count = 0;
		for (const auto& item: m_Downloads)
		{
			if (item->IsRunning())
			{
				count++;
			}
		}
		return count;
	}

	DownloadItem& IDownloadManager::AddDownload(std::unique_ptr<DownloadItem> download)
	{
		DownloadItem& ref = *m_Downloads.emplace_back(std::move(download));
		ref.SetWaiting();
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

	void IDownloadManager::LoadDownloads()
	{
		PauseAllActive();
		m_Downloads.clear();

		KxFileFinder finder(GetDownloadsLocation(), wxS("*"));
		for (KxFileItem fileItem = finder.FindNext(); fileItem.IsOK(); fileItem = finder.FindNext())
		{
			if (fileItem.IsNormalItem() && fileItem.IsFile())
			{
				DownloadItem& download = *m_Downloads.emplace_back(std::make_unique<DownloadItem>());
				if (!download.Load(fileItem))
				{
					download.LoadDefault(fileItem);
					download.Save();
				}
			}
		}
	}
	void IDownloadManager::SaveDownloads()
	{
		for (const auto& download: m_Downloads)
		{
			if (!download->IsRunning())
			{
				download->Save();
			}
		}
	}
	void IDownloadManager::PauseAllActive()
	{
		for (const auto& download: m_Downloads)
		{
			if (download->IsRunning())
			{
				download->Pause();
			}
		}
	}
	
	void IDownloadManager::ShowHiddenDownloads(bool show)
	{
		m_ShowHiddenDownloads = show;
		IEvent::MakeQueue<DownloadEvent>(DownloadEvent::EvtRefreshItems);
	}
	void IDownloadManager::SetDownloadsLocation(const wxString& location)
	{
		m_Location = location;

		KxFile(location).CreateFolder();
		LoadDownloads();
	}
	void IDownloadManager::SetMaxConcurrentDownloads(int count)
	{
		m_MaxConcurrentDownloads = count;
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

	bool IDownloadManager::TryQueueDownloadLink(const KxURI& link)
	{
		if (link)
		{
			for (ModNetworkRepository* repository: INetworkManager::GetInstance()->GetModRepositories())
			{
				if (repository->QueueDownload(link))
				{
					return true;
				}
			}
		}
		return false;
	}
}
