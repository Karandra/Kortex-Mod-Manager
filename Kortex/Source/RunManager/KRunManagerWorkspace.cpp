#include "stdafx.h"
#include "KRunManagerWorkspace.h"
#include "KRunManagerWorkspaceView.h"
#include "KThemeManager.h"
#include "KApp.h"
#include "KAux.h"

KxSingletonPtr_Define(KRunManagerWorkspace);

KRunManagerWorkspace::KRunManagerWorkspace(KMainWindow* mainWindow)
	:KWorkspace(mainWindow), m_ProgramListViewOptions(this, "ProgramListView")
{
	m_MainSizer = new wxBoxSizer(wxVERTICAL);
	CreateItemInManagersMenu();
}
KRunManagerWorkspace::~KRunManagerWorkspace()
{
	if (IsWorkspaceCreated())
	{
		KProgramOptionSerializer::SaveDataViewLayout(m_ViewModel->GetView(), m_ProgramListViewOptions);
	}
}
bool KRunManagerWorkspace::OnCreateWorkspace()
{
	m_ViewModel = new KRunManagerWorkspaceView();
	m_ViewModel->Create(this, m_MainSizer);

	KProgramOptionSerializer::LoadDataViewLayout(m_ViewModel->GetView(), m_ProgramListViewOptions);
	return true;
}

bool KRunManagerWorkspace::OnOpenWorkspace()
{
	size_t sel = m_ViewModel->GetRow(m_ViewModel->GetView()->GetSelection());
	m_ViewModel->RefreshItems();
	m_ViewModel->SelectItem(sel);
	return true;
}
bool KRunManagerWorkspace::OnCloseWorkspace()
{
	return true;
}
void KRunManagerWorkspace::OnReloadWorkspace()
{
}

wxString KRunManagerWorkspace::GetID() const
{
	return "KRunManagerWorkspace";
}
wxString KRunManagerWorkspace::GetName() const
{
	return T("ToolBar.RunManager");
}
