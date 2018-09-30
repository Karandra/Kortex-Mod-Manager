#include "stdafx.h"
#include "KWorkspaceController.h"
#include "KWorkspace.h"
#include "KMainWindow.h"
#include "KApp.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxString.h>

wxDEFINE_EVENT(KEVT_CONTROLLER_CHANGED, wxNotifyEvent);
wxDEFINE_EVENT(KEVT_CONTROLLER_SAVED, wxNotifyEvent);
wxDEFINE_EVENT(KEVT_CONTROLLER_DISCARDED, wxNotifyEvent);
wxDEFINE_EVENT(KEVT_CONTROLLER_SELECTED, wxNotifyEvent);

KApp& KWorkspaceController::GetApp()
{
	return KApp::Get();
}

wxString KWorkspaceController::GetSaveConfirmationCaption() const
{
	return wxString::Format("%s - %s", GetWorkspace()->GetName(), KxString::ToLower(T("Controller.SaveChanges.Caption")));
}
wxString KWorkspaceController::GetSaveConfirmationMessage() const
{
	return T("Controller.SaveChanges.Message");
}

KWorkspaceController::KWorkspaceController(KWorkspace* workspace)
	:m_Workspace(workspace)
{
}
KWorkspaceController::~KWorkspaceController()
{
}

bool KWorkspaceController::IsOK() const
{
	return m_Workspace != NULL;
}

KWorkspace* KWorkspaceController::GetWorkspace() const
{
	return m_Workspace;
}
KMainWindow* KWorkspaceController::GetMainWindow() const
{
	return m_Workspace->GetMainWindow();
}

wxWindowID KWorkspaceController::AskForSave(bool canCancel)
{
	if (HasUnsavedChanges())
	{
		KxTaskDialog dialog(GetMainWindow(), KxID_NONE, GetSaveConfirmationCaption(), GetSaveConfirmationMessage(), canCancel ? KxBTN_CANCEL : KxBTN_NONE);
		dialog.SetMainIcon(KxICON_WARNING);
		dialog.AddButton(KxID_YES, T("Controller.SaveChanges.Save"));
		dialog.AddButton(KxID_NO, T("Controller.SaveChanges.Discard"));

		wxWindowID ret = dialog.ShowModal();
		if (ret == KxID_YES)
		{
			SaveChanges();
		}
		else if (ret == KxID_NO)
		{
			DiscardChanges();
		}
		else
		{
			return KxID_CANCEL;
		}
	}
	return KxID_OK;
}
