#include "stdafx.h"
#include "Workspace.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameConfig.hpp>
#include <Kortex/Utility.hpp>

namespace Kortex::GameConfig
{
	Workspace::Workspace(KMainWindow* mainWindow)
		:KWorkspace(mainWindow)
	{
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
	}
	Workspace::~Workspace()
	{
	}
	bool Workspace::OnCreateWorkspace()
	{
		m_DisplayModel.CreateView(this, m_MainSizer);
		m_DisplayModel.LoadView();

		// Buttons
		m_SaveButton = new KxButton(this, KxID_NONE, KTr("Controller.SaveChanges.Save"));
		m_SaveButton->Bind(wxEVT_BUTTON, &Workspace::OnSaveButton, this);
		m_SaveButton->SetBitmap(KGetBitmap(KIMG_DISK));
		m_SaveButton->Disable();

		m_DiscardButton = new KxButton(this, KxID_NONE, KTr("Controller.SaveChanges.Discard"));
		m_DiscardButton->Bind(wxEVT_BUTTON, &Workspace::OnDiscardButton, this);
		m_DiscardButton->SetBitmap(KGetBitmap(KIMG_CROSS_WHITE));
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
		KMainWindow::GetInstance()->ClearStatus(1);
		return true;
	}
	void Workspace::OnReloadWorkspace()
	{
		m_DisplayModel.LoadView();
	}

	void Workspace::OnSaveButton(wxCommandEvent& event)
	{
		IGameConfigManager::GetInstance()->SaveChanges();
	}
	void Workspace::OnDiscardButton(wxCommandEvent& event)
	{
		IGameConfigManager::GetInstance()->DiscardChanges();
		m_DisplayModel.GetView()->Refresh();
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
	}
	void Workspace::OnChangesDiscarded()
	{
		m_SaveButton->Disable();
		m_DiscardButton->Disable();
	}
}
