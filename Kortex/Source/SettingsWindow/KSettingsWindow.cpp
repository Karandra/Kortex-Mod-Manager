#include "stdafx.h"
#include "KSettingsWindow.h"
#include "UI/KMainWindow.h"
#include <Kortex/Application.hpp>
#include <Kortex/ModManager.hpp>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxButton.h>
#include <KxFramework/KxTreeList.h>

using namespace Kortex;

wxWindow* KSettingsWindow::GetDialogMainCtrl() const
{
	return m_Workspace;
}
void KSettingsWindow::OnPrepareUninstall(wxCommandEvent& event)
{
	KxTaskDialog askDialog(this, KxID_NONE, KTrf("Settings.PrepareUninstall.Caption", Kortex::IApplication::GetInstance()->GetName()), KTr("Settings.PrepareUninstall.Message"), KxBTN_YES|KxBTN_NO, KxICON_WARNING);
	if (askDialog.ShowModal() == KxID_YES)
	{
		if (Kortex::IApplication::GetInstance()->Uninstall())
		{
			KxTaskDialog dialog(this, KxID_NONE, KTr("Settings.PrepareUninstall.Success"), wxEmptyString, KxBTN_NONE, KxICON_INFORMATION);
			dialog.AddButton(KxID_OK, KTr("Settings.PrepareUninstall.RebootNow"));
			dialog.AddButton(KxID_CANCEL, KTr("Settings.PrepareUninstall.RebootLater"));
			if (dialog.ShowModal() == KxID_OK)
			{
				wxShutdown(wxSHUTDOWN_REBOOT);
				m_MainWindow->Close(true);
			}
		}
		else
		{
			KxTaskDialog(this, KxID_NONE, KTr("Settings.PrepareUninstall.Error"), wxEmptyString, KxBTN_OK, KxICON_ERROR).ShowModal();
		}
	}
}

KSettingsWindow::KSettingsWindow(KMainWindow* mainWindow)
	:m_MainWindow(mainWindow)
{
	if (Create(mainWindow, KxID_NONE, KTr("Settings.Caption"), wxDefaultPosition, mainWindow->GetMinSize(), KxBTN_OK|KxBTN_CANCEL))
	{
		AddButton(KxID_REMOVE, KTr("Settings.PrepareUninstall.Button"), true).GetControl()
			->Bind(wxEVT_BUTTON, &KSettingsWindow::OnPrepareUninstall, this);

		IThemeManager::GetActive().ProcessWindow(m_ContentPanel);
		m_Workspace = new KSettingsWorkspace(this, mainWindow);

		PostCreate();
		SetMainIcon(KxICON_NONE);
	}
}
KSettingsWindow::~KSettingsWindow()
{
}

//////////////////////////////////////////////////////////////////////////
void KSettingsWorkspace::CreateControllerView()
{
	m_ControllerView = new KxTreeList(this, KxID_NONE, KxTreeList::DefaultStyle|KxTL_DCLICK_EXPAND|KxTL_FIX_FIRST_COLUMN|wxTL_SINGLE);
	m_ControllerView->SetRowHeight(23);

	/* Columns */
	const int flags = wxCOL_SORTABLE|wxCOL_RESIZABLE;
	const wxAlignment alignment = wxALIGN_LEFT;
	m_ControllerView->AddColumn(KTr("ConfigManager.View.Name"), 300, alignment, flags);
	m_ControllerView->AddColumn(KTr("ConfigManager.View.Value"), 300, alignment, flags);
	m_ControllerView->SetSortColumn(0, true);

	m_MainSizer->Add(m_ControllerView, 1, wxEXPAND);
}

KSettingsWorkspace::KSettingsWorkspace(KSettingsWindow* settingsWindow, KMainWindow* mainWindow)
	:KWorkspace(mainWindow, settingsWindow->GetContentWindow()), m_SettingsWindow(settingsWindow)
{
	m_MainSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_MainSizer);

	OnCreateWorkspace();
	OnOpenWorkspace();
}
KSettingsWorkspace::~KSettingsWorkspace()
{
	if (IsWorkspaceCreated())
	{
		OnCloseWorkspace();
		//KProgramOptionSerializer::SaveDataViewLayout(m_ControllerView->GetDataView(), m_AppConfigViewOptions);
	}
}
bool KSettingsWorkspace::OnCreateWorkspace()
{
	/* List */
	CreateControllerView();

	/* Controls */
	m_SaveButton = m_SettingsWindow->GetButton(KxID_OK);
	m_SaveButton->Bind(wxEVT_BUTTON, &KSettingsWorkspace::OnSaveButton, this);

	m_DiscardButton = m_SettingsWindow->GetButton(KxID_CANCEL);
	m_DiscardButton->Bind(wxEVT_BUTTON, &KSettingsWorkspace::OnDiscardButton, this);

	//KProgramOptionSerializer::LoadDataViewLayout(m_ControllerView->GetDataView(), m_AppConfigViewOptions);
	return true;
}

bool KSettingsWorkspace::OnOpenWorkspace()
{
	return true;
}
bool KSettingsWorkspace::OnCloseWorkspace()
{
	return true;
}

void KSettingsWorkspace::OnSaveButton(wxCommandEvent& event)
{
	#if 0
	if (m_Controller->HasUnsavedChanges())
	{
		m_Controller->SaveChanges();
		KxTaskDialog(m_SettingsWindow, KxID_NONE, KTr("Settings.SaveMessage"), wxEmptyString, KxBTN_OK, KxICON_INFORMATION).ShowModal();
	}
	#endif
	m_SettingsWindow->Close();
}
void KSettingsWorkspace::OnDiscardButton(wxCommandEvent& event)
{
	//m_Controller->DiscardChanges();
	m_SettingsWindow->Close();
}
void KSettingsWorkspace::OnControllerSaveDiscard(wxNotifyEvent& event)
{
}
