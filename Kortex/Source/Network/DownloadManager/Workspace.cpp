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
#include <KxFramework/KxComboBox.h>

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
	void Workspace::OnSelectConcurrentDownloadsCount(wxCommandEvent& event)
	{
		IDownloadManager::GetInstance()->SetMaxConcurrentDownloads(event.GetSelection());
		IEvent::MakeSend<DownloadEvent>(DownloadEvent::EvtConcurrentDownloadsCountChanged);
	}

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

		// Main view
		m_DisplayModel = new DisplayModel();
		m_DisplayModel->CreateView(this);

		m_MainSizer->Add(m_DisplayModel->GetView(), 1, wxEXPAND);
		GetDisplayModelOptions().LoadDataViewLayout(m_DisplayModel->GetView());

		// Toolbar
		m_ToolBar = new KxAuiToolBar(this, KxID_NONE, KxAuiToolBar::DefaultStyle|wxAUI_TB_PLAIN_BACKGROUND);
		m_ToolBar->SetBackgroundColour(GetBackgroundColour());
		m_ToolBar->AddStretchSpacer();

		{
			m_ToolBar->AddLabel(KTr("DownloadManager.ConcurrentDownloads.Label") + ':');

			KxComboBox* comboBox = new KxComboBox(m_ToolBar, KxID_NONE);
			comboBox->Bind(wxEVT_COMBOBOX, &Workspace::OnSelectConcurrentDownloadsCount, this);

			comboBox->AddItem(KTr("DownloadManager.ConcurrentDownloads.Unlimited"));
			for (int i = 1; i <= 10; i++)
			{
				comboBox->AddItem(KxString::Format(wxS("%1"), i));
			}

			comboBox->SetSelection(0);
			if (IDownloadManager::GetInstance()->HasConcurrentDownloadsLimit())
			{
				comboBox->SetSelection(IDownloadManager::GetInstance()->GetMaxConcurrentDownloads());
			}
			m_ToolBar->AddControl(comboBox);
		}

		m_ToolBar->UpdateUI();
		m_MainSizer->Add(m_ToolBar, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);
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
