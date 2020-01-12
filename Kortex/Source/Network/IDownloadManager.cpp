#include "stdafx.h"
#include "IDownloadManager.h"
#include <Kortex/NetworkManager.hpp>
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>
#include "Utility/Common.h"
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxDrive.h>
#include <KxFramework/KxRegistry.h>
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxUtility.h>

namespace
{
	template<class T> auto GetDownloadIterator(T&& items, const Kortex::DownloadItem& item)
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
		m_ShowHiddenDownloads = GetAInstanceOption(OName::ShowHidden).GetValueBool(m_ShowHiddenDownloads);
		m_MaxConcurrentDownloads = GetAInstanceOption(OName::MaxConcurrentDownloads).GetValueInt(m_MaxConcurrentDownloads);

		KxFile(GetDownloadsLocation()).CreateFolder();
	}
	void IDownloadManager::OnExit()
	{
		using namespace Application;
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
											KTr("DownloadManager.DownloadLocation.Invalid"),
											KxICON_ERROR
				);
				break;
			}
			case LocationStatus::InsufficientVolumeSpace:
			{
				INotificationCenter::Notify(KTr("DownloadManager.DownloadLocation"),
											KTr("DownloadManager.DownloadLocation.InsufficientSpace"),
											KxICON_ERROR
				);
				break;
			}
			case LocationStatus::InsufficientVolumeCapabilities:
			{
				INotificationCenter::Notify(KTr("DownloadManager.DownloadLocation"),
											KTr("DownloadManager.DownloadLocation.InsufficientCapabilities"),
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

		BroadcastProcessor::Get().ProcessEvent(DownloadEvent::EvtAdded, ref);
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
				KxFile(download.GetLocalPath()).RemoveFile(true);

				// Erase the item
				auto temp = std::move(*it);
				m_Downloads.erase(it);

				BroadcastProcessor::Get().ProcessEvent(DownloadEvent::EvtRemoved, *temp);
				return true;
			}
		}
		return false;
	}

	void IDownloadManager::LoadDownloads()
	{
		PauseAllActive();
		m_Downloads.clear();

		const wxString tempExt = wxS("tmp");
		KxFileFinder finder(GetDownloadsLocation(), wxS("*"));

		for (KxFileItem fileItem = finder.FindNext(); fileItem.IsOK(); fileItem = finder.FindNext())
		{
			if (fileItem.IsNormalItem() && fileItem.IsFile())
			{
				if (!Utility::SingleFileExtensionMatches(fileItem.GetFileExtension(), tempExt))
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
		BroadcastProcessor::Get().ProcessEvent(DownloadEvent::EvtRefreshItems);
	}

	wxString IDownloadManager::GetDownloadsLocation() const
	{
		using namespace Application;
		return GetAInstanceOption(OName::Downloads).GetAttribute(OName::Location);
	}
	void IDownloadManager::SetDownloadsLocation(const wxString& location)
	{
		using namespace Application;
		GetAInstanceOption(OName::Downloads).SetAttribute(OName::Location, location);
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
	bool IDownloadManager::AutoRenameIncrement(DownloadItem& item) const
	{
		size_t count = 0;
		while (FindDownloadByFileName(item.GetName(), &item))
		{
			if (item.ChangeFileName(RenameIncrement(item.GetName())))
			{
				count++;
			}
		}

		return count != 0;
	}

	bool IDownloadManager::QueueUnknownDownload(const wxString& link)
	{
		for (ModNetworkRepository* repository: INetworkManager::GetInstance()->GetModRepositories())
		{
			if (repository->QueueDownload(link))
			{
				return true;
			}
		}
		return QueueSimpleDownload(link);
	}
}
