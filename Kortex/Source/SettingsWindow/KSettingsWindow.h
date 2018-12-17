#pragma once
#include "stdafx.h"
#include "UI/KWorkspace.h"
#include <KxFramework/KxStdDialog.h>
class KxTreeList;
class KxButton;
class KMainWindow;
enum KImageEnum;

class KSettingsWorkspace;
class KSettingsWindow: public KxStdDialog
{
	private:
		KMainWindow* m_MainWindow = nullptr;
		KSettingsWorkspace* m_Workspace = nullptr;

	private:
		virtual int GetViewSizerProportion() const override
		{
			return 1;
		}
		virtual wxOrientation GetViewLabelSizerOrientation() const override
		{
			return wxHORIZONTAL;
		}
		virtual bool IsEnterAllowed(wxKeyEvent& event, wxWindowID* id) const override
		{
			return true;
		}
		virtual wxWindow* GetDialogMainCtrl() const override;

		void OnPrepareUninstall(wxCommandEvent& event);

	public:
		KSettingsWindow(KMainWindow* mainWindow);
		virtual ~KSettingsWindow();
};

class KSettingsWindowController;
class KSettingsWorkspace: public KWorkspace
{
	private:
		wxBoxSizer* m_MainSizer = nullptr;
		KSettingsWindow* m_SettingsWindow = nullptr;
		KSettingsWindowController* m_Controller = nullptr;
		KxTreeList* m_ControllerView = nullptr;
		KxButton* m_SaveButton = nullptr;
		KxButton* m_DiscardButton = nullptr;

	private:
		void CreateControllerView();

		virtual bool OnOpenWorkspace() override;
		virtual bool OnCloseWorkspace() override;

		void OnSaveButton(wxCommandEvent& event);
		void OnDiscardButton(wxCommandEvent& event);
		void OnControllerSaveDiscard(wxNotifyEvent& event);

	public:
		KSettingsWorkspace(KSettingsWindow* settingsWindow, KMainWindow* mainWindow);
		virtual ~KSettingsWorkspace();
		virtual bool OnCreateWorkspace() override;

	public:
		virtual wxString GetID() const override
		{
			return "AppSettings";
		}
		virtual wxString GetName() const override
		{
			return KTr("Settings.Caption");
		}
		virtual KImageEnum GetImageID() const override
		{
			return KIMG_GEAR;
		}
		virtual wxSizer* GetWorkspaceSizer() const override
		{
			return m_MainSizer;
		}
		virtual bool CanReload() const override
		{
			return false;
		}

		KSettingsWindow* GetSettingsWindow() const
		{
			return m_SettingsWindow;
		}
};
