#include "stdafx.h"
#include "KDownloadManager.h"
#include "KDownloadWorkspace.h"
#include "KDownloadView.h"
#include "Network/KNetwork.h"
#include "Network/KNetworkProviderNexus.h"
#include "NotificationCenter/KNotificationCenter.h"
#include "UI/KMainWindow.h"
#include "GameInstance/KGameInstance.h"
#include "KAux.h"
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxDataView.h>
#include <KxFramework/KxRegistry.h>
#include <KxFramework/KxLibrary.h>

namespace
{
	KDownloadView* GetViewAndItem(const KDownloadEntry& entry, KxDataViewItem& item)
	{
		KDownloadWorkspace* workspace = KDownloadWorkspace::GetInstance();
		if (workspace)
		{
			KDownloadView* view = workspace->GetModelView();
			if (view)
			{
				item = view->FindItem(entry);
				return view;
			}
		}

		item = KxDataViewItem();
		return NULL;
	}
}

KGameID KDownloadManager::TranslateNxmGameID(const wxString& id)
{
	const wxString idLower = KxString::ToLower(id);
	if (!idLower.IsEmpty())
	{
		// TES
		if (idLower == "morrowind")
		{
			return KGameIDs::Morrowind;
		}
		if (idLower == "oblivion")
		{
			return KGameIDs::Oblivion;
		}
		if (idLower == "skyrim")
		{
			return KGameIDs::Skyrim;
		}
		if (idLower == "skyrimse")
		{
			return KGameIDs::SkyrimSE;
		}

		// Fallout
		if (idLower == "fallout3")
		{
			return KGameIDs::Fallout3;
		}
		if (idLower == "falloutnv")
		{
			return KGameIDs::FalloutNV;
		}
		if (idLower == "fallout4")
		{
			return KGameIDs::Fallout4;
		}
	}
	return KGameIDs::NullGameID;
}
bool KDownloadManager::CheckCmdLineArgs(const wxCmdLineParser& args, wxString& link)
{
	return args.Found("NXM", &link);
}

KWorkspace* KDownloadManager::CreateWorkspace(KMainWindow* mainWindow)
{
	return new KDownloadWorkspace(mainWindow, this);
}

void KDownloadManager::OnChangeEntry(const KDownloadEntry& entry, bool noSerialize) const
{
	if (!noSerialize)
	{
		entry.Serialize();
	}

	if (m_IsReady)
	{
		KxDataViewItem item;
		KDownloadView* view = GetViewAndItem(entry, item);
		view->ItemChanged(item);
	}
}
void KDownloadManager::OnAddEntry(const KDownloadEntry& entry) const
{
	entry.Serialize();

	if (m_IsReady)
	{
		KxDataViewItem item;
		KDownloadView* view = GetViewAndItem(entry, item);
		view->ItemAdded(item);
	}
}
void KDownloadManager::OnRemoveEntry(const KDownloadEntry& entry) const
{
	entry.Serialize();

	if (m_IsReady)
	{
		KxDataViewItem item;
		KDownloadView* view = GetViewAndItem(entry, item);
		view->ItemDeleted(item);
	}
}

void KDownloadManager::OnDownloadComplete(KDownloadEntry& entry)
{
	OnChangeEntry(entry);
	KNotificationCenter::GetInstance()->Notify(this, TF("DownloadManager.Notification.DownloadCompleted").arg(entry.GetFileInfo().GetName()), KxICON_INFORMATION);
}
void KDownloadManager::OnDownloadPaused(KDownloadEntry& entry)
{
	OnChangeEntry(entry);
}
void KDownloadManager::OnDownloadStopped(KDownloadEntry& entry)
{
	OnChangeEntry(entry);
}
void KDownloadManager::OnDownloadResumed(KDownloadEntry& entry)
{
	OnChangeEntry(entry);
}
void KDownloadManager::OnDownloadFailed(KDownloadEntry& entry)
{
	OnChangeEntry(entry);
	KNotificationCenter::GetInstance()->Notify(this, TF("DownloadManager.Notification.DownloadFailed").arg(entry.GetFileInfo().GetName()), KxICON_WARNING);
}

bool KDownloadManager::CheckIsAssociatedWithNXM() const
{
	wxString path = KxRegistry::GetValue(KxREG_HKEY_CLASSES_ROOT, "NXM\\shell\\open\\command", "", KxREG_VALUE_SZ, KxREG_NODE_SYS, true).As<wxString>();
	return KxLibrary(NULL).GetFileName() == path.AfterFirst('"').BeforeFirst('"');
}

KDownloadManager::KDownloadManager()
	:m_Options(this)
{
	m_IsAssociatedWithNXM = CheckIsAssociatedWithNXM();
}
KDownloadManager::~KDownloadManager()
{
	m_IsReady = false;
}

void KDownloadManager::AssociateWithNXM()
{
	wxString appPath = KxLibrary(NULL).GetFileName();

	auto SetValue = [&appPath](const wxString& name, bool protocol = false)
	{
		if (protocol)
		{
			KxRegistry::SetValue(KxREG_HKEY_CLASSES_ROOT, name, "", "URL:NXM Protocol", KxREG_VALUE_SZ);
			KxRegistry::SetValue(KxREG_HKEY_CLASSES_ROOT, name, "URL Protocol", "URL:NXM Protocol", KxREG_VALUE_SZ);
		}
		else
		{
			KxRegistry::SetValue(KxREG_HKEY_CLASSES_ROOT, name, "", KApp::Get().GetAppDisplayName() + " Package", KxREG_VALUE_SZ);
		}
		KxRegistry::SetValue(KxREG_HKEY_CLASSES_ROOT, name + "\\DefaultIcon", "", appPath, KxREG_VALUE_SZ);
		KxRegistry::SetValue(KxREG_HKEY_CLASSES_ROOT, name + "\\shell\\open\\command", "", wxString::Format("\"%s\" \"-NXM %%1\"", appPath), KxREG_VALUE_SZ);
	};

	SetValue("NXM", true);
	SetValue("NXM_File_Type", false);
	m_IsAssociatedWithNXM = CheckIsAssociatedWithNXM();
}

void KDownloadManager::LoadDownloads()
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
				auto& entry = m_Downloads.emplace_back(new KDownloadEntry());

				// Try to load from download xml or load as much as possible from download itself
				if (!entry->DeSerialize(xmlPath, item))
				{
					entry->DeSerializeDefault(item);
					entry->Serialize();
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
void KDownloadManager::SaveDownloads()
{
	for (const auto& entry: m_Downloads)
	{
		if (!entry->IsRunning())
		{
			entry->Serialize();
		}
	}
}
void KDownloadManager::OnShutdown()
{
	PauseAllActive();
}
void KDownloadManager::PauseAllActive()
{
	for (const auto& entry: KDownloadManager::GetInstance()->GetDownloads())
	{
		if (entry->IsRunning())
		{
			entry->Pause();
		}
	}
}

wxString KDownloadManager::GetDownloadsLocation() const
{
	wxString location = m_Options.GetAttribute("DownloadsLocation");
	if (location.IsEmpty())
	{
		location = KGameInstance::GetActive()->GetInstanceRelativePath("Downloads");
		KxFile(location).CreateFolder();
	}
	return location;
}
void KDownloadManager::SetDownloadsLocation(const wxString& location)
{
	m_Options.SetAttribute("DownloadsLocation", location);
	KxFile(location).CreateFolder();
}

KDownloadEntry::RefVector KDownloadManager::GetNotRunningDownloads(bool installedOnly) const
{
	KDownloadEntry::RefVector items;
	for (const auto& entry: m_Downloads)
	{
		if (!entry->IsRunning())
		{
			if (installedOnly && !entry->IsInstalled())
			{
				continue;
			}
			items.push_back(*entry);
		}
	}
	return items;
}
KDownloadEntry* KDownloadManager::FindDownloadByFileName(const wxString& name, const KDownloadEntry* except) const
{
	const wxString nameL = KxString::ToLower(name);
	for (const auto& entry: m_Downloads)
	{
		if (entry.get() != except && KxString::ToLower(entry->GetFileInfo().GetName()) == nameL)
		{
			return entry.get();
		}
	}
	return NULL;
}
wxString KDownloadManager::RenameIncrement(const wxString& name) const
{
	wxRegEx regEx(u8R"((.*)\((\d+)\)\.(.*))", wxRE_DEFAULT|wxRE_ADVANCED|wxRE_ICASE);
	if (regEx.Matches(name))
	{
		int64_t value = 0;
		if (regEx.GetMatch(name, 2).ToLongLong(&value))
		{
			value++;

			wxString newName = name;
			regEx.Replace(&newName, wxString::Format("\\1(%lld).\\3", value));
			return newName;
		}
	}
	return wxString::Format("%s (1).%s", name.BeforeLast('.'), name.AfterLast('.'));
}
void KDownloadManager::AutoRenameIncrement(KDownloadEntry& entry) const
{
	while (FindDownloadByFileName(entry.GetFileInfo().GetName(), &entry))
	{
		entry.GetFileInfo().SetName(RenameIncrement(entry.GetFileInfo().GetName()));
	}
}

bool KDownloadManager::RemoveDownload(KDownloadEntry& download)
{
	if (!download.IsRunning())
	{
		auto it = GetDownloadIterator(download);
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
bool KDownloadManager::QueueDownload(const KNetworkProvider::DownloadInfo& downloadInfo,
									 const KNetworkProvider::FileInfo& fileInfo,
									 const KNetworkProvider* provider,
									 const KGameID& id
)
{
	if (downloadInfo.IsOK() && fileInfo.IsOK())
	{
		KDownloadEntry& entry = *m_Downloads.emplace_back(std::make_unique<KDownloadEntry>(downloadInfo, fileInfo, provider, id));
		AutoRenameIncrement(entry);
		
		OnAddEntry(entry);
		entry.Run();
		return true;
	}
	return false;
}
bool KDownloadManager::QueueFromOutside(const wxString& link)
{
	if (KDownloadManager::GetInstance()->QueueNXM(link))
	{
		KDownloadWorkspace* workspace = KDownloadWorkspace::GetInstance();
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
bool KDownloadManager::QueueNXM(const wxString& link)
{
	wxRegEx reg(u8R"(nxm:\/\/(\w+)\/mods\/(\d+)\/files\/(\d+))", wxRE_EXTENDED|wxRE_ADVANCED|wxRE_ICASE);
	if (reg.Matches(link))
	{
		KGameID game = TranslateNxmGameID(reg.GetMatch(link, 1));
		int64_t modID = -1;
		int64_t fileID = -1;
		reg.GetMatch(link, 2).ToLongLong(&modID);
		reg.GetMatch(link, 3).ToLongLong(&fileID);

		if (game.IsOK() && modID != -1 && fileID != -1)
		{
			KNetworkProviderNexus* nexus = KNetworkProviderNexus::GetInstance();
			auto fileInfo = nexus->GetFileInfo(modID, fileID, game);
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

wxString KDownloadManager::GetID() const
{
	return "KDownloadManager";
}
wxString KDownloadManager::GetName() const
{
	return T("DownloadManager.Name");
}
wxString KDownloadManager::GetVersion() const
{
	return "1.0";
}
