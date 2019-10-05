#include "stdafx.h"
#include "Workspace.h"
#include "WorkspaceController.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameConfig.hpp>
#include <Kortex/Utility.hpp>
#include <KxFramework/KxTaskDialog.h>

namespace Kortex::GameConfig
{
	void Workspace::OnSaveButton(wxCommandEvent& event)
	{
		IGameConfigManager::GetInstance()->SaveChanges();
	}
	void Workspace::OnDiscardButton(wxCommandEvent& event)
	{
		IGameConfigManager::GetInstance()->DiscardChanges();
	}

	bool Workspace::OnCreateWorkspace()
	{
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
		SetSizer(m_MainSizer);

		m_DisplayModel.CreateView(this, m_MainSizer);
		m_DisplayModel.LoadView();

		// Buttons
		m_SaveButton = new KxButton(this, KxID_NONE, KTr("Controller.SaveChanges.Save"));
		m_SaveButton->Bind(wxEVT_BUTTON, &Workspace::OnSaveButton, this);
		m_SaveButton->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Disk));
		m_SaveButton->Disable();

		m_DiscardButton = new KxButton(this, KxID_NONE, KTr("Controller.SaveChanges.Discard"));
		m_DiscardButton->Bind(wxEVT_BUTTON, &Workspace::OnDiscardButton, this);
		m_DiscardButton->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::CrossWhite));
		m_DiscardButton->Disable();

		wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
		m_MainSizer->Add(buttonSizer, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);

		buttonSizer->AddStretchSpacer();
		buttonSizer->Add(m_SaveButton);
		buttonSizer->Add(m_DiscardButton, 0, wxLEFT, KLC_HORIZONTAL_SPACING_SMALL);
		return true;
	}
	bool Workspace::OnOpenWorkspace()
	{
		return true;
	}
	bool Workspace::OnCloseWorkspace()
	{
		// Workspace controller
		/*
		if (GetWorkspaceController()->AskForSave() == KxID_OK)
		{
			IMainWindow::GetInstance()->ClearStatus(1);
			return true;
		}
		*/
		return true;
	}
	void Workspace::OnReloadWorkspace()
	{
		m_DisplayModel.LoadView();
	}
	
	Workspace::Workspace()
		:m_DisplayModel(*IGameConfigManager::GetInstance())
	{
	}
	Workspace::~Workspace()
	{
	}

	wxString Workspace::GetID() const
	{
		return "ConfigManager::Workspace";
	}
	wxString Workspace::GetName() const
	{
		return GameConfigModule::GetInstance()->GetModuleInfo().GetName();
	}
	wxString Workspace::GetNameShort() const
	{
		return GameConfigModule::GetInstance()->GetModuleInfo().GetName();
	}

	void Workspace::OnChangesMade()
	{
		m_SaveButton->Enable();
		m_DiscardButton->Enable();
	}
	void Workspace::OnChangesSaved()
	{
		m_SaveButton->Disable();
		m_DiscardButton->Disable();
		m_DisplayModel.RefreshView();
	}
	void Workspace::OnChangesDiscarded()
	{
		// Same logic
		OnChangesSaved();
	}
}
