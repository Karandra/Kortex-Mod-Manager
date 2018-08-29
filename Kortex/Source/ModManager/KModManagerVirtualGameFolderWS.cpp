#include "stdafx.h"
#include "KModManagerVirtualGameFolderWS.h"

KxSingletonPtr_Define(KModManagerVirtualGameFolderWS);

KModManagerVirtualGameFolderWS::KModManagerVirtualGameFolderWS(KMainWindow* mainWindow)
	:KWorkspace(mainWindow), m_OptionsUI(this, "MainUI"), m_ModListViewOptions(this, "VirtualGameFolderView")
{
	m_MainSizer = new wxBoxSizer(wxVERTICAL);
}
KModManagerVirtualGameFolderWS::~KModManagerVirtualGameFolderWS()
{
	if (IsWorkspaceCreated())
	{
	}
}
bool KModManagerVirtualGameFolderWS::OnCreateWorkspace()
{
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
