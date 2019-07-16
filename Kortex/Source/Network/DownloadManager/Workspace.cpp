#include "stdafx.h"
#include "Workspace.h"
#include "DisplayModel.h"
#include <Kortex/Application.hpp>
#include <Kortex/Notification.hpp>
#include <Kortex/DownloadManager.hpp>
#include "Utility/KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>

namespace
{
	auto GetDisplayModelOptions()
	{
		using namespace Kortex;
		using namespace Kortex::Application;

		return GetAInstanceOptionOf<IDownloadManager>(OName::Workspace, OName::DisplayModel);
	}
}

namespace Kortex::DownloadManager
{
	Workspace::Workspace(KMainWindow* mainWindow)
		:KWorkspace(mainWindow)
	{
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
	}
	Workspace::~Workspace()
	{
		if (IsWorkspaceCreated())
		{
			GetDisplayModelOptions().SaveDataViewLayout(m_ViewModel->GetView());
		}
	}
	bool Workspace::OnCreateWorkspace()
	{
		IDownloadManager* manager = IDownloadManager::GetInstance();
		manager->LoadDownloads();

		m_ViewModel = new DisplayModel();
		m_ViewModel->Create(this, m_MainSizer);
		GetDisplayModelOptions().LoadDataViewLayout(m_ViewModel->GetView());

		return true;
	}

	bool Workspace::OnOpenWorkspace()
	{
		// This will show notification about invalid download location 
		IDownloadManager::GetInstance()->OnAccessDownloadLocation();

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
