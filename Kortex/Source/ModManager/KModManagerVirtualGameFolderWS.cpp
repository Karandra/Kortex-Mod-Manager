#include "stdafx.h"
#include "KModManagerVirtualGameFolderWS.h"
#include "KModManagerVirtualGameFolderModel.h"
#include <KxFramework/KxSearchBox.h>

KxSingletonPtr_Define(KModManagerVirtualGameFolderWS);

KModManagerVirtualGameFolderWS::KModManagerVirtualGameFolderWS(KMainWindow* mainWindow)
	:KWorkspace(mainWindow), m_OptionsUI(this, "MainUI"), m_ViewOptions(this, "View")
{
	m_MainSizer = new wxBoxSizer(wxVERTICAL);
}
KModManagerVirtualGameFolderWS::~KModManagerVirtualGameFolderWS()
{
	if (IsWorkspaceCreated())
	{
		KProgramOptionSerializer::SaveDataViewLayout(m_Model->GetView(), m_ViewOptions);
	}
}
bool KModManagerVirtualGameFolderWS::OnCreateWorkspace()
{
	m_Model = new KModManagerVirtualGameFolderModel();
	m_Model->Create(this, m_MainSizer);
	m_Model->RefreshItems();

	m_SearchBox = new KxSearchBox(this, wxID_NONE);
	m_SearchBox->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &KModManagerVirtualGameFolderWS::OnModSerach, this);
	m_SearchBox->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &KModManagerVirtualGameFolderWS::OnModSerach, this);
	m_MainSizer->Add(m_SearchBox, 0, wxTOP|wxEXPAND, KLC_VERTICAL_SPACING);

	KProgramOptionSerializer::LoadDataViewLayout(m_Model->GetView(), m_ViewOptions);
	return true;
}

bool KModManagerVirtualGameFolderWS::OnOpenWorkspace()
{
	return true;
}
bool KModManagerVirtualGameFolderWS::OnCloseWorkspace()
{
	return true;
}
void KModManagerVirtualGameFolderWS::OnReloadWorkspace()
{
	m_Model->RefreshItems();
}

void KModManagerVirtualGameFolderWS::OnModSerach(wxCommandEvent& event)
{
	if (m_Model->SetSearchMask(event.GetEventType() == wxEVT_SEARCHCTRL_SEARCH_BTN ? event.GetString() : wxEmptyString))
	{
		m_Model->RefreshItems();
	}
}

wxString KModManagerVirtualGameFolderWS::GetID() const
{
	return "KModManagerVirtualGameFolderWS";
}
wxString KModManagerVirtualGameFolderWS::GetName() const
{
	return T("VirtualGameFolderWS.Name");
}
wxString KModManagerVirtualGameFolderWS::GetNameShort() const
{
	return T("VirtualGameFolderWS.NameShort");
}
