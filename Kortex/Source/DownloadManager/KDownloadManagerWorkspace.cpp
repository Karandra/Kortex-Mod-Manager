#include "stdafx.h"
#include "KDownloadManagerWorkspace.h"
#include "KDownloadManagerView.h"
#include "KDownloadManager.h"
#include "KThemeManager.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>

KxSingletonPtr_Define(KDownloadManagerWorkspace);

bool KDownloadManagerWorkspace::CheckDownloadLocation()
{
	wxString sLocation = KDownloadManager::GetInstance()->GetDownloadsLocation();
	if (sLocation.IsEmpty() || !KxFile(sLocation).IsFolderExist())
	{
		KxTaskDialog messageDialog(this, KxID_NONE, T("DownloadManager.DownloadLocationInaccessible"), wxEmptyString, KxBTN_OK, KxICON_WARNING);
		messageDialog.ShowModal();

		KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_OPEN_FOLDER);
		if (dialog.ShowModal() == KxID_OK)
		{
			KDownloadManager::GetInstance()->SetDownloadsLocation(dialog.GetResult());
			return true;
		}
	}
	return false;
}

KDownloadManagerWorkspace::KDownloadManagerWorkspace(KMainWindow* mainWindow, KDownloadManager* manager)
	:KWorkspace(mainWindow), m_ViewOptions(this, "DownloadsListView")
{
	m_MainSizer = new wxBoxSizer(wxVERTICAL);
}
KDownloadManagerWorkspace::~KDownloadManagerWorkspace()
{
	KDownloadManager::GetInstance()->SetReady(false);
	if (IsWorkspaceCreated())
	{
		KProgramOptionSerializer::SaveDataViewLayout(m_ViewModel->GetView(), m_ViewOptions);
	}
}
bool KDownloadManagerWorkspace::OnCreateWorkspace()
{
	KDownloadManager* manager = KDownloadManager::GetInstance();
	manager->LoadDownloads();

	m_ViewModel = new KDownloadManagerView();
	m_ViewModel->Create(this, m_MainSizer);

	KProgramOptionSerializer::LoadDataViewLayout(m_ViewModel->GetView(), m_ViewOptions);
	KDownloadManager::GetInstance()->SetReady();

	wxString downloadLink;
	if (manager->CheckCmdLineArgs(KApp::Get().GetCmdLineParser(), downloadLink))
	{
		manager->QueueFromOutside(downloadLink);
	}
	return true;
}

bool KDownloadManagerWorkspace::OnOpenWorkspace()
{
	if (CheckDownloadLocation())
	{
		CallAfter([this]()
		{
			ScheduleRefresh();
		});
	}
	return true;
}
bool KDownloadManagerWorkspace::OnCloseWorkspace()
{
	return true;
}
void KDownloadManagerWorkspace::OnReloadWorkspace()
{
	KDownloadManager::GetInstance()->LoadDownloads();
	m_ViewModel->RefreshItems();
}

wxString KDownloadManagerWorkspace::GetID() const
{
	return "KDownloadManagerWorkspace";
}
wxString KDownloadManagerWorkspace::GetName() const
{
	return T("DownloadManager.Name");
}
wxString KDownloadManagerWorkspace::GetNameShort() const
{
	return T("DownloadManager.NameShort");
}
