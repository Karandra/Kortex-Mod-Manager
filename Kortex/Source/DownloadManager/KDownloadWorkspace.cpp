#include "stdafx.h"
#include "KDownloadWorkspace.h"
#include "KDownloadView.h"
#include "KDownloadManager.h"
#include "KThemeManager.h"
#include "NotificationCenter/KNotificationCenter.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>

KDownloadWorkspace::KDownloadWorkspace(KMainWindow* mainWindow, KDownloadManager* manager)
	:KWorkspace(mainWindow), m_ViewOptions(this, "DownloadsListView")
{
	m_MainSizer = new wxBoxSizer(wxVERTICAL);
}
KDownloadWorkspace::~KDownloadWorkspace()
{
	KDownloadManager::GetInstance()->SetReady(false);
	if (IsWorkspaceCreated())
	{
		KProgramOptionSerializer::SaveDataViewLayout(m_ViewModel->GetView(), m_ViewOptions);
	}
}
bool KDownloadWorkspace::OnCreateWorkspace()
{
	KDownloadManager* manager = KDownloadManager::GetInstance();
	manager->LoadDownloads();

	m_ViewModel = new KDownloadView();
	m_ViewModel->Create(this, m_MainSizer);

	KProgramOptionSerializer::LoadDataViewLayout(m_ViewModel->GetView(), m_ViewOptions);
	KDownloadManager::GetInstance()->SetReady();

	CallAfter([]()
	{
		wxString downloadLink;
		if (KDownloadManager::GetInstance()->CheckCmdLineArgs(KApp::Get().GetCmdLineParser(), downloadLink))
		{
			KDownloadManager::GetInstance()->QueueFromOutside(downloadLink);
		}
	});
	return true;
}

bool KDownloadWorkspace::OnOpenWorkspace()
{
	return true;
}
bool KDownloadWorkspace::OnCloseWorkspace()
{
	return true;
}
void KDownloadWorkspace::OnReloadWorkspace()
{
	KDownloadManager::GetInstance()->LoadDownloads();
	m_ViewModel->RefreshItems();
}

wxString KDownloadWorkspace::GetID() const
{
	return "KDownloadManagerWorkspace";
}
wxString KDownloadWorkspace::GetName() const
{
	return KTr("DownloadManager.Name");
}
wxString KDownloadWorkspace::GetNameShort() const
{
	return KTr("DownloadManager.NameShort");
}
