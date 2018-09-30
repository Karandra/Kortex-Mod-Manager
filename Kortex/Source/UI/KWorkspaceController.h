#pragma once
#include "stdafx.h"
#include "KImageProvider.h"
class KMainWindow;
class KWorkspace;
class KApp;

wxDECLARE_EVENT(KEVT_CONTROLLER_CHANGED, wxNotifyEvent);
wxDECLARE_EVENT(KEVT_CONTROLLER_SAVED, wxNotifyEvent);
wxDECLARE_EVENT(KEVT_CONTROLLER_DISCARDED, wxNotifyEvent);
wxDECLARE_EVENT(KEVT_CONTROLLER_SELECTED, wxNotifyEvent);

class KWorkspaceController: public wxEvtHandler
{
	private:
		KWorkspace* m_Workspace = NULL;

	protected:
		KApp& GetApp();
		virtual void ResetView() {};

		virtual wxString GetSaveConfirmationCaption() const;
		virtual wxString GetSaveConfirmationMessage() const;

	public:
		KWorkspaceController(KWorkspace* workspace);
		virtual ~KWorkspaceController();

	public:
		virtual bool IsOK() const;

		virtual KWorkspace* GetWorkspace() const;
		virtual KMainWindow* GetMainWindow() const;

		virtual wxWindowID AskForSave(bool canCancel = true);
		virtual bool HasUnsavedChanges() const = 0;
		virtual void SaveChanges() = 0;
		virtual void DiscardChanges() = 0;

		virtual void Reload() {};
		virtual void LoadView() {};
};
