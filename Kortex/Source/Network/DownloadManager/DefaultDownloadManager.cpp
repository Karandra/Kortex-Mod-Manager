#include "stdafx.h"
#include "DefaultDownloadManager.h"
#include "DefaultDownloadEntry.h"
#include "DisplayModel.h"
#include "Workspace.h"
#include "Network/ModNetwork/Nexus.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/Notification.hpp>
#include <Kortex/GameInstance.hpp>
#include "UI/KMainWindow.h"
#include "Utility/KAux.h"
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxDataView.h>
#include <KxFramework/KxRegistry.h>
#include <KxFramework/KxLibrary.h>

namespace
{
	using namespace Kortex;
	using namespace Kortex::DownloadManager;

	DisplayModel* GetViewAndItem(const IDownloadEntry& entry, KxDataViewItem& item)
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

	void DefaultDownloadManager::OnChangeEntry(const IDownloadEntry& entry, bool noSave) const
	{
		if (!noSave)
		{
			entry.Save();
		}

		if (m_IsReady)
		{
			KxDataViewItem item;
			DisplayModel* view = GetViewAndItem(entry, item);
			view->ItemChanged(item);
		}
	}
	void DefaultDownloadManager::OnAddEntry(const IDownloadEntry& entry) const
	{
		entry.Save();

		if (m_IsReady)
		{
			KxDataViewItem item;
			DisplayModel* view = GetViewAndItem(entry, item);
			view->ItemAdded(item);
		}
	}
	void DefaultDownloadManager::OnRemoveEntry(const IDownloadEntry& entry) const
	{
		entry.Save();

		if (m_IsReady)
		{
			KxDataViewItem item;
			DisplayModel* view = GetViewAndItem(entry, item);
			view->ItemDeleted(item);
		}
	}

	void DefaultDownloadManager::OnDownloadComplete(IDownloadEntry& entry)
	{
		OnChangeEntry(entry);
		INotificationCenter::GetInstance()->Notify(KTr("DownloadManager.Notification.DownloadCompleted"), KTrf("DownloadManager.Notification.DownloadCompletedEx", entry.GetFileInfo().Name), KxICON_INFORMATION);
	}
	void DefaultDownloadManager::OnDownloadPaused(IDownloadEntry& entry)
	{
		OnChangeEntry(entry);
	}
	void DefaultDownloadManager::OnDownloadStopped(IDownloadEntry& entry)
	{
		OnChangeEntry(entry);
	}
	void DefaultDownloadManager::OnDownloadResumed(IDownloadEntry& entry)
	{
		OnChangeEntry(entry);
	}
	void DefaultDownloadManager::OnDownloadFailed(IDownloadEntry& entry)
	{
		OnChangeEntry(entry);
		INotificationCenter::GetInstance()->Notify(KTr("DownloadManager.Notification.DownloadFailed"), KTrf("DownloadManager.Notification.DownloadFailedEx", entry.GetFileInfo().Name), KxICON_WARNING);
	}

	void DefaultDownloadManager::OnInit()
	{
		m_IsReady = true;
		m_IsAssociatedWithNXM = CheckIsAssociatedWithNXM();

		KxFile(GetDownloadsLocation()).CreateFolder();
	}
	void DefaultDownloadManager::OnExit()
	{
		m_IsReady = false;
		PauseAllActive();
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
		KxFileItem item = finder.FindNext();
		while (item.IsOK())
		{
			if (item.IsNormalItem() && item.IsFile() && !item.GetName().IsEmpty())
			{
				// Load everything that's not XML file
				if (!KAux::IsSingleFileExtensionMatches(item.GetName(), wxS("xml")))
				{
					const wxString xmlPath = item.GetFullPath() + wxS(".xml");
					auto& entry = m_Downloads.emplace_back(new DefaultDownloadEntry());

					// Try to load from download xml or load as much as possible from download itself
					if (!entry->Load(xmlPath, item))
					{
						entry->LoadDefault(item);
						entry->Save();
					}
					if (entry->IsHidden() && !showHidden)
					{
						m_Downloads.pop_back();
					}
				}
			}
			item = finder.FindNext();
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
	auto DefaultDownloadManager::OnAccessDownloadLocation() const -> DownloadLocationError
	{
		const DownloadLocationError status = CheckDownloadLocation(GetDownloadsLocation());
		switch (status)
		{
			case DownloadLocationError::NotExist:
			case DownloadLocationError::NotSpecified:
			{
				INotificationCenter::GetInstance()->Notify(KTr("DownloadManager.DownloadLocation"),
														   KTr("DownloadManager.DownloadLocationInvalid"),
														   KxICON_ERROR
				);
				break;
			}
			case DownloadLocationError::InsufficientVolumeSpace:
			{
				INotificationCenter::GetInstance()->Notify(KTr("DownloadManager.DownloadLocation"),
														   KTr("DownloadManager.DownloadLocationInsufficientSpace"),
														   KxICON_ERROR
				);
				break;
			}
			case DownloadLocationError::InsufficientVolumeCapabilities:
			{
				INotificationCenter::GetInstance()->Notify(KTr("DownloadManager.DownloadLocation"),
														   KTr("DownloadManager.DownloadLocationInsufficientCapabilities"),
														   KxICON_ERROR
				);
				break;
			}
		};
		return status;
	}

	IDownloadEntry& DefaultDownloadManager::NewDownload()
	{
		return *m_Downloads.emplace_back(std::make_unique<DefaultDownloadEntry>());
	}
	bool DefaultDownloadManager::RemoveDownload(IDownloadEntry& download)
	{
		if (!download.IsRunning())
		{
			auto it = GetDownloadIterator(m_Downloads, download);
			if (it != m_Downloads.end())
			{
				if (KxFile(download.GetFullPath()).RemoveFile(true) && KxFile(download.GetMetaFilePath()).RemoveFile(true))
				{
					m_Downloads.erase(it);
					return true;
				}
			}
		}
		return false;
	}
	
	bool DefaultDownloadManager::QueueDownload(ModNetworkRepository& modRepository,
											   const ModDownloadReply& downloadInfo,
											   const ModFileReply& fileInfo,
											   const GameID& id
	)
	{
		if (OnAccessDownloadLocation() != DownloadLocationError::Success)
		{
			return false;
		}

		if (downloadInfo.IsOK() && fileInfo.IsOK())
		{
			IDownloadEntry& entry = *m_Downloads.emplace_back(std::make_unique<DefaultDownloadEntry>(downloadInfo, fileInfo, modRepository, id));
			AutoRenameIncrement(entry);

			OnAddEntry(entry);
			entry.Run();
			return true;
		}
		return false;
	}
	bool DefaultDownloadManager::TryQueueDownloadLink(const wxString& link)
	{
		if (QueueNXM(link))
		{
			Workspace* workspace = Workspace::GetInstance();
			workspace->SwitchHere();
			return true;
		}
		return false;
	}
	
	// IDownloadManagerNXM
	bool DefaultDownloadManager::CheckIsAssociatedWithNXM() const
	{
		wxString path = KxRegistry::GetValue(KxREG_HKEY_CLASSES_ROOT, "NXM\\shell\\open\\command", "", KxREG_VALUE_SZ, KxREG_NODE_SYS, true).As<wxString>();
		return IApplication::GetInstance()->GetExecutablePath() == path.AfterFirst('"').BeforeFirst('"');
	}
	bool DefaultDownloadManager::IsAssociatedWithNXM() const
	{
		return m_IsAssociatedWithNXM;
	}
	void DefaultDownloadManager::AssociateWithNXM()
	{
		wxString appPath = KxLibrary(nullptr).GetFileName();

		auto SetValue = [&appPath](const wxString& name, bool protocol = false)
		{
			if (protocol)
			{
				KxRegistry::SetValue(KxREG_HKEY_CLASSES_ROOT, name, "", "URL:NXM Protocol", KxREG_VALUE_SZ);
				KxRegistry::SetValue(KxREG_HKEY_CLASSES_ROOT, name, "URL Protocol", "URL:NXM Protocol", KxREG_VALUE_SZ);
			}
			else
			{
				KxRegistry::SetValue(KxREG_HKEY_CLASSES_ROOT, name, "", IApplication::GetInstance()->GetName() + " Package", KxREG_VALUE_SZ);
			}
			KxRegistry::SetValue(KxREG_HKEY_CLASSES_ROOT, name + "\\DefaultIcon", "", appPath, KxREG_VALUE_SZ);
			KxRegistry::SetValue(KxREG_HKEY_CLASSES_ROOT, name + "\\shell\\open\\command", "", wxString::Format("\"%s\" \"-NXM %%1\"", appPath), KxREG_VALUE_SZ);
		};

		SetValue("NXM", true);
		SetValue("NXM_File_Type", false);
		m_IsAssociatedWithNXM = CheckIsAssociatedWithNXM();
	}

	bool DefaultDownloadManager::QueueNXM(const wxString& link)
	{
		using namespace NetworkManager;

		if (NexusModNetwork* nexus = NexusModNetwork::GetInstance())
		{
			GameID gameID;
			NetworkModInfo modInfo;
			NexusNXMLinkData nxmExtraInfo;

			if (nexus->ParseNXM(link, gameID, modInfo, nxmExtraInfo))
			{
				ModNetworkRepository& repository = nexus->GetComponent<ModNetworkRepository>();

				if (auto fileInfo = repository.GetModFileInfo(ModRepositoryRequest(modInfo, gameID)))
				{
					ModRepositoryRequest request(modInfo, gameID);
					request.SetExtraInfo(nxmExtraInfo);

					if (auto linkItems = repository.GetFileDownloads(request); !linkItems.empty())
					{
						// Here we should actually select preferred download server based on user chose if we got more than one,
						// but for now just use the first one.
						return QueueDownload(repository, linkItems.front(), *fileInfo, gameID);
					}
					return false;
				}
			}
		}
		return false;
	}
}
