#include "stdafx.h"
#include "KWorkspace.h"
#include "KWorkspaceController.h"
#include "KMainWindow.h"
#include "KThemeManager.h"
#include "GameInstance/KInstnaceManagement.h"
#include "KEvents.h"
#include "KApp.h"

bool KWorkspace::OnOpenWorkspaceInternal()
{
	if (m_IsReloadSheduled)
	{
		m_IsReloadSheduled = false;
		ReloadWorkspace();
	}
	
	RefreshWindowTitle();
	if (OnOpenWorkspace())
	{
		m_IsFirstTimeOpen = false;
		return true;
	}
	return false;
}
bool KWorkspace::OnCloseWorkspaceInternal()
{
	return OnCloseWorkspace();
}
bool KWorkspace::OnCreateWorkspaceInternal()
{
	if (IsWorkspaceCreated())
	{
		return true;
	}
	else
	{
		m_CreatingWorkspace = true;
		if (OnCreateWorkspace())
		{
			m_QSM = GetQuickSettingsMenu();

			m_CreatingWorkspace = false;
			m_IsCreated = true;
			m_Sizer->Add(GetWorkspaceSizer(), 1, wxEXPAND|wxALL, std::min(KLC_HORIZONTAL_SPACING, KLC_VERTICAL_SPACING));
			Layout();
			return true;
		}
	}
	return false;
}
void KWorkspace::Init()
{
	m_Sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_Sizer);
}

bool KWorkspace::MakeSubWorkspace(KWorkspace* workspace)
{
	if (workspace->IsSubWorkspace())
	{
		KThemeManager::Get().ProcessWindow(workspace);
		return workspace->Reparent(GetSubWorkspaceContainer());
	}
	return false;
}

wxString KWorkspace::OnGetWindowTitle() const
{
	return wxString::Format("%s – %s – %s", KApp::Get().GetAppDisplayName(), KGameInstance::GetActive()->GetShortName(), GetName());
}
bool KWorkspace::OnOpenWorkspace()
{
	return true;
}
bool KWorkspace::OnCloseWorkspace()
{
	return true;
}

KxAuiToolBarItem* KWorkspace::CreateToolBarButton()
{
	m_ToolBarButton = KMainWindow::CreateToolBarButton(GetMainWindow()->GetMainToolBar(), GetName(), GetImageID());
	m_ToolBarButton->Bind(KxEVT_AUI_TOOLBAR_CLICK, &KWorkspace::SwitchHereEvent, this);

	return m_ToolBarButton;
}
KxMenuItem* KWorkspace::CreateItemInManagersMenu()
{
	m_ManagersMenuItem = GetMainWindow()->GetManagersMenu()->Add(new KxMenuItem(GetName()));
	m_ManagersMenuItem->SetBitmap(KGetBitmap(GetImageID()));
	m_ManagersMenuItem->Bind(KxEVT_MENU_SELECT, &KWorkspace::SwitchHereEvent, this);

	return m_ManagersMenuItem;
}

KWorkspace::KWorkspace(KMainWindow* mainWindow, wxWindow* parentWindow)
	:KxPanel(parentWindow, KxID_NONE), m_MainWindow(mainWindow)
{
	Init();
}
KWorkspace::KWorkspace(KMainWindow* mainWindow)
	:KxPanel(mainWindow->GetWorkspaceContainer(), KxID_NONE), m_MainWindow(mainWindow)
{
	Init();
}
KWorkspace::~KWorkspace()
{
	delete m_WorkspaceController;
	delete m_QSM;
}

void KWorkspace::SwitchHereEvent(wxNotifyEvent& event)
{
	SwitchHere();
}

int KWorkspace::GetWorkspaceIndex() const
{
	if (!IsSubWorkspace())
	{
		return m_MainWindow->GetWorkspaceContainer()->FindPage(this);
	}
	return wxNOT_FOUND;
}

void KWorkspace::RefreshWindowTitle()
{
	if (!IsSubWorkspace())
	{
		m_MainWindow->SetTitle(OnGetWindowTitle());
	}
}
bool KWorkspace::IsWorkspaceVisible() const
{
	if (IsSubWorkspace())
	{
		return IsShown();
	}
	else
	{
		return m_MainWindow->GetCurrentWorkspace() == this;
	}
}
bool KWorkspace::ReloadWorkspace()
{
	if (IsWorkspaceCreated() || m_CreatingWorkspace)
	{
		OnReloadWorkspace();
		return true;
	}
	return false;
}
bool KWorkspace::SwitchHere()
{
	if (IsSubWorkspace())
	{
		wxBookCtrlBase* tabs = dynamic_cast<wxBookCtrlBase*>(GetParent());
		if (tabs)
		{
			tabs->SetSelection(GetTabIndex());
		}
	}
	else
	{
		return GetMainWindow()->SwitchWorkspace(this);
	}
	return false;
}
bool KWorkspace::CreateNow()
{
	if (!IsWorkspaceCreated())
	{
		return OnCreateWorkspaceInternal();
	}
	return true;
}
void KWorkspace::ScheduleReload()
{
	CallAfter([this]()
	{
		if (IsWorkspaceCreated())
		{
			if (IsWorkspaceVisible())
			{
				ReloadWorkspace();
			}
			else
			{
				// ReloadWorkspace will be called when workspace is opened next time
				m_IsReloadSheduled = true;
			}
		}
		else
		{
			CreateNow();
		}
	});
}
