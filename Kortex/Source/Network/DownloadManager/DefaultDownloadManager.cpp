#include "stdafx.h"
#include "DefaultDownloadManager.h"
#include "DefaultDownloadEntry.h"
#include "DisplayModel.h"
#include "Workspace.h"
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/Notification.hpp>
#include <Kortex/GameInstance.hpp>
#include "UI/KMainWindow.h"
#include "KAux.h"
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
		INotificationCenter::GetInstance()->Notify(KTr("DownloadManager.Notification.DownloadCompleted"), KTrf("DownloadManager.Notification.DownloadCompletedEx", entry.GetFileInfo().GetName()), KxICON_INFORMATION);
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
		INotificationCenter::GetInstance()->Notify(KTr("DownloadManager.Notification.DownloadFailed"), KTrf("DownloadManager.Notification.DownloadFailedEx", entry.GetFileInfo().GetName()), KxICON_WARNING);
	}

	void DefaultDownloadManager::OnInit()
	{
		m_IsReady = true;
		m_IsAssociatedWithNXM = CheckIsAssociatedWithNXM();
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

	wxString DefaultDownloadManager::GetDownloadsLocation() const
	{
		using namespace Application;
		
		auto option = GetAInstanceOption(OName::Downloads);
		wxString location = option.GetAttribute(OName::Location);
		if (location.IsEmpty())
		{
			location = IGameInstance::GetActive()->GetInstanceRelativePath("Downloads");
			option.SetAttribute(OName::Location, location);
			KxFile(location).CreateFolder();
		}
		return location;
	}
	void DefaultDownloadManager::SetDownloadsLocation(const wxString& location)
	{
		using namespace Application;

		GetAInstanceOption(OName::Downloads).SetAttribute(OName::Location, location);
		KxFile(location).CreateFolder();
	}

	IDownloadEntry& DefaultDownloadManager::NewDownload()
	{
		return *m_Downloads.emplace_back(new DefaultDownloadEntry());
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
	bool DefaultDownloadManager::QueueDownload(const Network::DownloadInfo& downloadInfo,
											   const Network::FileInfo& fileInfo,
											   const INetworkProvider* provider,
											   const GameID& id
	)
	{
		if (downloadInfo.IsOK() && fileInfo.IsOK())
		{
			IDownloadEntry& entry = *m_Downloads.emplace_back(std::make_unique<DefaultDownloadEntry>(downloadInfo, fileInfo, provider, id));
			AutoRenameIncrement(entry);

			OnAddEntry(entry);
			entry.Run();
			return true;
		}
		return false;
	}
	bool DefaultDownloadManager::QueueFromOutside(const wxString& link)
	{
		if (QueueNXM(link))
		{
			Workspace* workspace = Workspace::GetInstance();
			workspace->SwitchHere();

			KMainWindow* mainWindow = KMainWindow::GetInstance();
			if (mainWindow->IsIconized())
			{
				mainWindow->Restore();
			}
			if (!mainWindow->IsShownOnScreen() || !mainWindow->IsExposed(wxGetMousePosition()))
			{
				mainWindow->Show();
			}
			return true;
		}
		return false;
	}
	
	// IDownloadManagerNXM
	bool DefaultDownloadManager::CheckIsAssociatedWithNXM() const
	{
		wxString path = KxRegistry::GetValue(KxREG_HKEY_CLASSES_ROOT, "NXM\\shell\\open\\command", "", KxREG_VALUE_SZ, KxREG_NODE_SYS, true).As<wxString>();
		return KxLibrary(nullptr).GetFileName() == path.AfterFirst('"').BeforeFirst('"');
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

	GameID DefaultDownloadManager::TranslateGameID(const wxString& nexusID) const
	{
		const wxString idLower = KxString::ToLower(nexusID);
		if (!idLower.IsEmpty())
		{
			// TES
			if (idLower == "morrowind")
			{
				return GameIDs::Morrowind;
			}
			if (idLower == "oblivion")
			{
				return GameIDs::Oblivion;
			}
			if (idLower == "skyrim")
			{
				return GameIDs::Skyrim;
			}
			if (idLower == "skyrimse")
			{
				return GameIDs::SkyrimSE;
			}

			// Fallout
			if (idLower == "fallout3")
			{
				return GameIDs::Fallout3;
			}
			if (idLower == "falloutnv")
			{
				return GameIDs::FalloutNV;
			}
			if (idLower == "fallout4")
			{
				return GameIDs::Fallout4;
			}
		}
		return GameIDs::NullGameID;
	}
	bool DefaultDownloadManager::QueueNXM(const wxString& link)
	{
		wxRegEx reg(u8R"(nxm:\/\/(\w+)\/mods\/(\d+)\/files\/(\d+))", wxRE_ADVANCED|wxRE_ICASE);
		if (reg.Matches(link))
		{
			GameID game = TranslateGameID(reg.GetMatch(link, 1));
			int64_t modID = -1;
			int64_t fileID = -1;
			reg.GetMatch(link, 2).ToLongLong(&modID);
			reg.GetMatch(link, 3).ToLongLong(&fileID);

			if (game.IsOK() && modID != -1 && fileID != -1)
			{
				Network::NexusProvider* nexus = Network::NexusProvider::GetInstance();
				auto fileInfo = nexus->GetFileItem(modID, fileID, game);
				if (fileInfo.IsOK())
				{
					bool success = true;
					auto links = nexus->GetFileDownloadLinks(modID, fileID, game);
					for (const auto& v: links)
					{
						success = success && QueueDownload(v, fileInfo, nexus, game);
					}
					return success && !links.empty();
				}
			}
		}
		return false;
	}
}
