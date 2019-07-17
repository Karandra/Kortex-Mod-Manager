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
			GetDisplayModelOptions().SaveDataViewLayout(m_DisplayModel->GetView());
		}
	}
	bool Workspace::OnCreateWorkspace()
	{
		IDownloadManager* manager = IDownloadManager::GetInstance();
		manager->LoadDownloads();

		m_DisplayModel = new DisplayModel();
		m_DisplayModel->CreateView(this);
		m_MainSizer->Add(m_DisplayModel->GetView(), 1, wxEXPAND);

		GetDisplayModelOptions().LoadDataViewLayout(m_DisplayModel->GetView());

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
		m_DisplayModel->RefreshItems();
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
