#include "stdafx.h"
#include "KVirtualGameFolderWorkspace.h"
#include "KVirtualGameFolderModel.h"
#include "KEvents.h"
#include <KxFramework/KxSearchBox.h>

KVirtualGameFolderWorkspace::KVirtualGameFolderWorkspace(KMainWindow* mainWindow)
	:KWorkspace(mainWindow), m_OptionsUI(this, "MainUI"), m_ViewOptions(this, "View")
{
	KEvent::Bind(KEVT_MOD_VIRTUAL_TREE_INVALIDATED, &KVirtualGameFolderWorkspace::OnViewInvalidated, this);
	KEvent::Bind(KEVT_MOD_TOGGLED, &KVirtualGameFolderWorkspace::OnViewInvalidated, this);
	KEvent::Bind(KEVT_MODS_REORDERED, &KVirtualGameFolderWorkspace::OnViewInvalidated, this);

	m_MainSizer = new wxBoxSizer(wxVERTICAL);
}
KVirtualGameFolderWorkspace::~KVirtualGameFolderWorkspace()
{
	if (IsWorkspaceCreated())
	{
		KProgramOptionSerializer::SaveDataViewLayout(m_Model->GetView(), m_ViewOptions);
	}
}
bool KVirtualGameFolderWorkspace::OnCreateWorkspace()
{
	m_Model = new KVirtualGameFolderModel();
	m_Model->Create(this, m_MainSizer);
	m_Model->RefreshItems();

	m_SearchBox = new KxSearchBox(this, wxID_NONE);
	m_SearchBox->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &KVirtualGameFolderWorkspace::OnModSerach, this);
	m_SearchBox->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &KVirtualGameFolderWorkspace::OnModSerach, this);
	m_MainSizer->Add(m_SearchBox, 0, wxTOP|wxEXPAND, KLC_VERTICAL_SPACING);

	KProgramOptionSerializer::LoadDataViewLayout(m_Model->GetView(), m_ViewOptions);
	return true;
}

bool KVirtualGameFolderWorkspace::OnOpenWorkspace()
{
	return true;
}
bool KVirtualGameFolderWorkspace::OnCloseWorkspace()
{
	return true;
}
void KVirtualGameFolderWorkspace::OnReloadWorkspace()
{
	m_Model->RefreshItems();
}

void KVirtualGameFolderWorkspace::OnModSerach(wxCommandEvent& event)
{
	if (m_Model->SetSearchMask(event.GetEventType() == wxEVT_SEARCHCTRL_SEARCH_BTN ? event.GetString() : wxEmptyString))
	{
		m_Model->RefreshItems();
	}
}
void KVirtualGameFolderWorkspace::OnViewInvalidated(KEvent& event)
{
	if (IsWorkspaceCreated() && IsWorkspaceVisible())
	{
		ScheduleReload();
	}
}

wxString KVirtualGameFolderWorkspace::GetID() const
{
	return "KModManagerVirtualGameFolderWS";
}
wxString KVirtualGameFolderWorkspace::GetName() const
{
	return T("VirtualGameFolderWS.Name");
}
wxString KVirtualGameFolderWorkspace::GetNameShort() const
{
	return T("VirtualGameFolderWS.NameShort");
}
