#include "stdafx.h"
#include "KProgramWorkspace.h"
#include "KProgramManagerModel.h"
#include "KEvents.h"
#include "KApp.h"
#include "KAux.h"

KProgramWorkspace::KProgramWorkspace(KMainWindow* mainWindow)
	:KWorkspace(mainWindow), m_ProgramListViewOptions(this, "ProgramListView")
{
	m_MainSizer = new wxBoxSizer(wxVERTICAL);
	CreateItemInManagersMenu();
}
KProgramWorkspace::~KProgramWorkspace()
{
	if (IsWorkspaceCreated())
	{
		KProgramOptionSerializer::SaveDataViewLayout(m_ViewModel->GetView(), m_ProgramListViewOptions);
	}
}
bool KProgramWorkspace::OnCreateWorkspace()
{
	m_ViewModel = new KProgramManagerModel();
	m_ViewModel->Create(this, m_MainSizer);
	KProgramOptionSerializer::LoadDataViewLayout(m_ViewModel->GetView(), m_ProgramListViewOptions);

	KEvent::Bind(KEVT_VFS_TOGGLED, &KProgramWorkspace::OnVFSToggled, this);
	return true;
}

bool KProgramWorkspace::OnOpenWorkspace()
{
	size_t sel = m_ViewModel->GetRow(m_ViewModel->GetView()->GetSelection());
	m_ViewModel->RefreshItems();
	m_ViewModel->SelectItem(sel);
	return true;
}
bool KProgramWorkspace::OnCloseWorkspace()
{
	return true;
}
void KProgramWorkspace::OnReloadWorkspace()
{
	m_ViewModel->RefreshItems();
}

void KProgramWorkspace::OnVFSToggled(KVFSEvent& event)
{
	ScheduleReload();
}

wxString KProgramWorkspace::GetID() const
{
	return "KProgramManagerWorkspace";
}
wxString KProgramWorkspace::GetName() const
{
	return T("ProgramManager.Name");
}
wxString KProgramWorkspace::GetNameShort() const
{
	return T("ProgramManager.NameShort");
}
