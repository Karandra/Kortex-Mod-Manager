#include "stdafx.h"
#include "Workspace.h"
#include "DisplayModel.h"
#include <Kortex/Application.hpp>
#include <Kortex/Notification.hpp>
#include <Kortex/DownloadManager.hpp>
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>

namespace Kortex::DownloadManager
{
	Workspace::Workspace(KMainWindow* mainWindow)
		:KWorkspace(mainWindow)//, m_ViewOptions(this, "DownloadsListView")
	{
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
	}
	Workspace::~Workspace()
	{
		if (IsWorkspaceCreated())
		{
			//KProgramOptionSerializer::SaveDataViewLayout(m_ViewModel->GetView(), m_ViewOptions);
		}
	}
	bool Workspace::OnCreateWorkspace()
	{
		IDownloadManager* manager = IDownloadManager::GetInstance();
		manager->LoadDownloads();

		m_ViewModel = new DisplayModel();
		m_ViewModel->Create(this, m_MainSizer);
		//KProgramOptionSerializer::LoadDataViewLayout(m_ViewModel->GetView(), m_ViewOptions);

		CallAfter([manager]()
		{
			wxString downloadLink;
			IDownloadManagerNXM* nxm = nullptr;
			if (manager->QueryInterface(nxm) && nxm->CheckCmdLineArgs(IApplication::GetInstance()->GetCmdLineParser(), downloadLink))
			{
				manager->QueueFromOutside(downloadLink);
			}
		});
		return true;
	}

	bool Workspace::OnOpenWorkspace()
	{
		return true;
	}
	bool Workspace::OnCloseWorkspace()
	{
		return true;
	}
	void Workspace::OnReloadWorkspace()
	{
		IDownloadManager::GetInstance()->LoadDownloads();
		m_ViewModel->RefreshItems();
	}

	wxString Workspace::GetID() const
	{
		return "KDownloadManagerWorkspace";
	}
	wxString Workspace::GetName() const
	{
		return KTr("DownloadManager.Name");
	}
	wxString Workspace::GetNameShort() const
	{
		return KTr("DownloadManager.NameShort");
	}
}
