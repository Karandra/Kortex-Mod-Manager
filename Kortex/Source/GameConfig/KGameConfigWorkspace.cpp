#include "stdafx.h"
#include "KGameConfigWorkspace.h"
#include "KGameConfigWorkspaceController.h"
#include "ConfigManager/KConfigManager.h"
#include "Profile/KProfile.h"
#include "KApp.h"

KxSingletonPtr_Define(KGameConfigWorkspace);

KGameConfigWorkspace::KGameConfigWorkspace(KMainWindow* mainWindow)
	:KWorkspace(mainWindow), m_GameConfigViewOptions(this, "GameConfigView")
{
	CreateToolBarButton();
	m_MainSizer = new wxBoxSizer(wxVERTICAL);
}

void KGameConfigWorkspace::CreateControllerView()
{
	m_ControllerView = new KxTreeList(this, KxID_NONE, KxTreeList::DefaultStyle|KxTL_DCLICK_EXPAND|KxTL_FIX_FIRST_COLUMN|wxTL_SINGLE);
	m_ControllerView->SetRowHeight(23);

	/* Columns */
	const int flags = wxCOL_SORTABLE|wxCOL_RESIZABLE|wxCOL_REORDERABLE;
	const wxAlignment alignment = wxALIGN_LEFT;
	m_ControllerView->AddColumn(T("ConfigManager.View.Path"), 275, alignment, flags);
	m_ControllerView->AddColumn(T("ConfigManager.View.Name"), 250, alignment, flags);
	m_ControllerView->AddColumn(T("ConfigManager.View.Type"), 75, alignment, flags);
	m_ControllerView->AddColumn(T("ConfigManager.View.Value"), 325, alignment, flags);
	m_ControllerView->SetSortColumn(0, true);
}
bool KGameConfigWorkspace::OnCreateWorkspace()
{
	m_ListViewSizer = new wxBoxSizer(wxHORIZONTAL);
	m_ControlsSizer = new wxBoxSizer(wxHORIZONTAL);
	m_MainSizer->Add(m_ListViewSizer, 1, wxEXPAND);
	m_MainSizer->Add(m_ControlsSizer, 0, wxEXPAND|wxALIGN_RIGHT|wxTOP, KLC_VERTICAL_SPACING);
	
	/* List */
	CreateControllerView();
	m_ListViewSizer->Add(m_ControllerView, 1, wxEXPAND, KLC_VERTICAL_SPACING);

	/* Controls */
	m_SaveButton = new KxButton(this, KxID_NONE, T(KxID_SAVE));
	m_SaveButton->SetBitmap(KGetBitmap(KIMG_DISK));
	m_SaveButton->Disable();
	m_SaveButton->Bind(wxEVT_BUTTON, &KGameConfigWorkspace::OnSaveButton, this);

	m_DiscardButton = new KxButton(this, KxID_NONE, T(KxID_UNDO));
	m_DiscardButton->SetBitmap(KGetBitmap(KIMG_CROSS_WHITE));
	m_DiscardButton->Disable();
	m_DiscardButton->Bind(wxEVT_BUTTON, &KGameConfigWorkspace::OnDiscardButton, this);

	m_ControlsSizer->AddStretchSpacer();
	m_ControlsSizer->Add(m_SaveButton, 0, wxLEFT, KLC_HORIZONTAL_SPACING_SMALL);
	m_ControlsSizer->Add(m_DiscardButton, 0, wxLEFT, KLC_HORIZONTAL_SPACING_SMALL);

	m_Controller = new KGameConfigWorkspaceController(this, m_ControllerView);
	m_Controller->Bind(KEVT_CONTROLLER_SAVED, &KGameConfigWorkspace::OnControllerSaveDiscard, this);
	m_Controller->Bind(KEVT_CONTROLLER_DISCARDED, &KGameConfigWorkspace::OnControllerSaveDiscard, this);
	m_Controller->Bind(KEVT_CONTROLLER_CHNAGED, [this](wxNotifyEvent& event)
	{
		m_SaveButton->Enable();
		m_DiscardButton->Enable();
	});
	m_Controller->LoadView();
	SetWorkspaceController(m_Controller);

	//KProgramOptionSerializer::LoadDataViewLayout(m_ControllerView->GetDataView(), m_GameConfigViewOptions);
	return true;
}
KGameConfigWorkspace::~KGameConfigWorkspace()
{
	if (IsWorkspaceCreated())
	{
		//KProgramOptionSerializer::SaveDataViewLayout(m_ControllerView->GetDataView(), m_GameConfigViewOptions);
	}
}

bool KGameConfigWorkspace::OnOpenWorkspace()
{
	return true;
}
bool KGameConfigWorkspace::OnCloseWorkspace()
{
	if (m_Controller->AskForSave() == KxID_OK)
	{
		GetMainWindow()->ClearStatus();
		return true;
	}
	return false;
}
void KGameConfigWorkspace::OnReloadWorkspace()
{
	m_Controller->Reload();
	m_Controller->LoadView();
}

wxString KGameConfigWorkspace::GetID() const
{
	return "KGameConfigWorkspace";
}
wxString KGameConfigWorkspace::GetName() const
{
	return T("ToolBar.ConfigManager");
}

void KGameConfigWorkspace::OnSaveButton(wxCommandEvent& event)
{
	m_Controller->SaveChanges();
}
void KGameConfigWorkspace::OnDiscardButton(wxCommandEvent& event)
{
	m_Controller->DiscardChanges();
}
void KGameConfigWorkspace::OnControllerSaveDiscard(wxNotifyEvent& event)
{
	m_SaveButton->Disable();
	m_DiscardButton->Disable();
}
