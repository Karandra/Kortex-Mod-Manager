#include "stdafx.h"
#include "KDownloadManager.h"
#include "KDownloadManagerWorkspace.h"
#include "KDownloadManagerView.h"
#include "Network/KNetwork.h"
#include "Network/KNetworkProviderNexus.h"
#include "UI/KMainWindow.h"
#include <KxFramework/KxFileFinder.h>
#include <KxFramework/KxDataView.h>
#include <KxFramework/KxRegistry.h>
#include <KxFramework/KxLibrary.h>

namespace
{
	KDownloadManagerView* GetViewAndItem(const KDownloadEntry& entry, KxDataViewItem& item)
	{
		KDownloadManagerWorkspace* workspace = KDownloadManagerWorkspace::GetInstance();
		if (workspace)
		{
			KDownloadManagerView* view = workspace->GetModelView();
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

KProfileID KDownloadManager::TranslateNxmGameID(const wxString& id)
{
	const wxString idLower = KxString::ToLower(id);
	if (!idLower.IsEmpty())
	{
		// TES
		if (idLower == "morrowind")
		{
			return KProfileIDs::Morrowind;
		}
		if (idLower == "oblivion")
		{
			return KProfileIDs::Oblivion;
		}
		if (idLower == "skyrim")
		{
			return KProfileIDs::Skyrim;
		}
		if (idLower == "skyrimse")
		{
			return KProfileIDs::SkyrimSE;
		}

		// Fallout
		if (idLower == "fallout3")
		{
			return KProfileIDs::Fallout3;
		}
		if (idLower == "falloutnv")
		{
			return KProfileIDs::FalloutNV;
		}
		if (idLower == "fallout4")
		{
			return KProfileIDs::Fallout4;
		}
	}
	return KProfileIDs::NullProfileID;
}
bool KDownloadManager::CheckCmdLineArgs(const wxCmdLineParser& args, wxString& link)
{
	return args.Found("NXM", &link);
}

KWorkspace* KDownloadManager::CreateWorkspace(KMainWindow* mainWindow)
{
	return new KDownloadManagerWorkspace(mainWindow, this);
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
		KDownloadManagerView* view = GetViewAndItem(entry, item);
		view->ItemChanged(item);
	}
}
void KDownloadManager::OnAddEntry(const KDownloadEntry& entry) const
{
	entry.Serialize();

	if (m_IsReady)
	{
		KxDataViewItem item;
		KDownloadManagerView* view = GetViewAndItem(entry, item);
		view->ItemAdded(item);
	}
}
void KDownloadManager::OnRemoveEntry(const KDownloadEntry& entry) const
{
	entry.Serialize();

	if (m_IsReady)
	{
		KxDataViewItem item;
		KDownloadManagerView* view = GetViewAndItem(entry, item);
		view->ItemDeleted(item);
	}
}

void KDownloadManager::OnDownloadComplete(KDownloadEntry& entry)
{
	OnChangeEntry(entry);
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
	OnChangeEntry(entry, true);
}
void KDownloadManager::OnDownloadFailed(KDownloadEntry& entry)
{
	OnChangeEntry(entry);
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

	KxFileFinder finder(GetDownloadsLocation(), "*.xml");
	KxFileFinderItem item = finder.FindNext();
	while (item.IsOK())
	{
		if (item.IsNormalItem() && item.IsFile() && !item.GetName().IsEmpty())
		{
			auto& entry = m_Downloads.emplace_back(new KDownloadEntry());
			if (!entry->DeSerialize(item.GetFullPath()) || (entry->IsHidden() && !showHidden))
			{
				m_Downloads.pop_back();
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

KDownloadEntry::RefContainer KDownloadManager::GetNotRunningItems(bool installedOnly) const
{
	KDownloadEntry::RefContainer items;
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
KDownloadEntry* KDownloadManager::FindEntryWithFileName(const wxString& name, const KDownloadEntry* except) const
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
	while (FindEntryWithFileName(entry.GetFileInfo().GetName(), &entry))
	{
		entry.GetFileInfo().SetName(RenameIncrement(entry.GetFileInfo().GetName()));
	}
}

bool KDownloadManager::RemoveDownload(KDownloadEntry& download)
{
	if (!download.IsRunning())
	{
		auto it = FindEntryIter(download);
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
									 const KProfileID& id
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
		KDownloadManagerWorkspace* workspace = KDownloadManagerWorkspace::GetInstance();
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
		KProfileID game = TranslateNxmGameID(reg.GetMatch(link, 1));
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
