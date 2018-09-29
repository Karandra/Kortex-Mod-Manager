#include "stdafx.h"
#include "KProgramManagerWorkspace.h"
#include "KProgramManagerModel.h"
#include "KThemeManager.h"
#include "KApp.h"
#include "KAux.h"

KProgramManagerWorkspace::KProgramManagerWorkspace(KMainWindow* mainWindow)
	:KWorkspace(mainWindow), m_ProgramListViewOptions(this, "ProgramListView")
{
	m_MainSizer = new wxBoxSizer(wxVERTICAL);
	CreateItemInManagersMenu();
}
KProgramManagerWorkspace::~KProgramManagerWorkspace()
{
	if (IsWorkspaceCreated())
	{
		KProgramOptionSerializer::SaveDataViewLayout(m_ViewModel->GetView(), m_ProgramListViewOptions);
	}
}
bool KProgramManagerWorkspace::OnCreateWorkspace()
{
	m_ViewModel = new KProgramManagerModel();
	m_ViewModel->Create(this, m_MainSizer);

	KProgramOptionSerializer::LoadDataViewLayout(m_ViewModel->GetView(), m_ProgramListViewOptions);
	return true;
}

bool KProgramManagerWorkspace::OnOpenWorkspace()
{
	size_t sel = m_ViewModel->GetRow(m_ViewModel->GetView()->GetSelection());
	m_ViewModel->RefreshItems();
	m_ViewModel->SelectItem(sel);
	return true;
}
bool KProgramManagerWorkspace::OnCloseWorkspace()
{
	return true;
}
void KProgramManagerWorkspace::OnReloadWorkspace()
{
}

wxString KProgramManagerWorkspace::GetID() const
{
	return "KProgramManagerWorkspace";
}
wxString KProgramManagerWorkspace::GetName() const
{
	return T("ProgramManager.Name");
}
