#include "stdafx.h"
#include "KMainWindow.h"
#include "SettingsWindow/KSettingsWindow.h"
#include "ModManager/KModManager.h"
#include "Network/KNetwork.h"
#include "DownloadManager/KDownloadManager.h"
#include "KInstanceSelectionDialog.h"
#include "KAboutDialog.h"
#include "KThemeManager.h"
#include "KWorkspace.h"
#include "KWorkspaceController.h"
#include "Events/KVFSEvent.h"
#include "GameInstance/KInstnaceManagement.h"
#include "GameInstance/Config/KLocationsManagerConfig.h"
#include "KPluggableManager.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxProcess.h>
#include <KxFramework/KxShell.h>

#include "ProgramManager/KProgramManager.h"
#include "GameConfig/KGameConfigWorkspace.h"
#include "ModManager/KModWorkspace.h"
#include "PackageManager/KPackageManagerWorkspace.h"
#include "PackageCreator/KPackageCreatorWorkspace.h"
#include "ProgramManager/KProgramManagerWorkspace.h"

KxAuiToolBarItem* KMainWindow::CreateToolBarButton(KxAuiToolBar* toolBar, const wxString& label, KImageEnum imageID, wxItemKind kind, int index)
{
	wxBitmap bitmap = wxNullBitmap;
	if (imageID != KIMG_NONE)
	{
		bitmap = KApp::Get().GetImageList()->GetBitmap(imageID);
	}

	KxAuiToolBarItem* button = toolBar->AddTool(label, bitmap, kind);
	if (!toolBar->HasFlag(wxAUI_TB_TEXT))
	{
		button->SetShortHelp(label);
	}
	
	#if 0
	if (imageHoverEffects && bitmap.IsOk())
	{
		wxImage temp = bitmap.ConvertToImage();
		button->SetHoverBitmap(wxBitmap(KAux::ChangeLightness(temp, 200), 32));
	}
	#endif

	return button;
}
wxSize KMainWindow::GetDialogBestSize(wxWindow* dialog)
{
	int screenWidth = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
	int screenHeight = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
	float scaleX = 0.85f;
	float scaleY = scaleX;

	return wxSize(screenWidth * scaleX, screenHeight * scaleY);
}
const void* KMainWindow::GetUniqueID()
{
	// Should not be zero
	return reinterpret_cast<const void*>(0xFF);
}

void KMainWindow::CreateToolBar()
{
	m_ToolBarSizer = new wxBoxSizer(wxHORIZONTAL);
	m_MainSizer->Add(m_ToolBarSizer, 0, wxEXPAND);

	auto NewToolBar = [this](int proportion = 1, bool hasText = false, int flags = 0)
	{
		flags |= KxAuiToolBar::DefaultStyle|wxAUI_TB_PLAIN_BACKGROUND|wxAUI_TB_HORIZONTAL|wxAUI_TB_HORZ_LAYOUT;
		if (hasText)
		{
			flags |= wxAUI_TB_TEXT;
		}

		KxAuiToolBar* toolBar = new KxAuiToolBar(this, KxID_NONE, flags);
		toolBar->SetToolBorderPadding(KLC_HORIZONTAL_SPACING_SMALL);
		toolBar->SetMargins(KLC_HORIZONTAL_SPACING, KLC_HORIZONTAL_SPACING, KLC_VERTICAL_SPACING, KLC_VERTICAL_SPACING + 1);
		KThemeManager::Get().ProcessWindow(toolBar);

		m_ToolBarSizer->Add(toolBar, proportion, wxEXPAND);
		return toolBar;
	};

	/* Main ToolBar */
	m_ToolBar = NewToolBar(1, true);
	{
		m_ToolBar_MainMenu = m_ToolBar->AddTool(wxString::Format("%8s%s%8s", "", "Kortex", ""), KGetBitmap(KIMG_APPLICATION_LOGO_SMALL), wxITEM_DROPDOWN);
		m_ToolBar_MainMenu->SetOptionEnabled(KxAUI_TBITEM_OPTION_LCLICK_MENU);
		m_ToolBar_InsertionIndex = m_ToolBar->AddSeparator()->GetIndex();
	}
	m_ToolBar->Realize();

	/* Quick ToolBar */
	m_QuickToolBar = NewToolBar(0, true);
	{
		m_QuickToolBar_Login = m_QuickToolBar->AddTool(wxEmptyString, KGetBitmap(KIMG_APPLICATION_LOGO_SMALL), wxITEM_NORMAL);
		m_QuickToolBar_Login->Bind(KxEVT_AUI_TOOLBAR_CLICK, &KNetwork::OnLoginButton, KNetwork::GetInstance());
		KNetwork::GetInstance()->SetLoginButton(m_QuickToolBar_Login);

		m_QuickToolBar->AddSeparator();

		m_QuickToolBar_QuickSettingsMenu = m_QuickToolBar->AddTool(wxEmptyString, KGetBitmap(KIMG_GEAR), wxITEM_NORMAL);
		m_QuickToolBar_QuickSettingsMenu->Bind(KxEVT_AUI_TOOLBAR_CLICK, &KMainWindow::OnQSMButton, this);

		m_QuickToolBar_Help = m_QuickToolBar->AddTool(wxEmptyString, KGetBitmap(KIMG_QUESTION_FRAME), wxITEM_NORMAL);
	}
	m_QuickToolBar->Realize();
}
void KMainWindow::CreateStatusBar()
{
	m_StatusBar = new KxStatusBarEx(this, KxID_NONE, 5);
	m_StatusBar->SetImageList(KGetImageList());
	m_StatusBar->SetStatusWidths({12, -3, -3, -1, 50});
	SetStatusBar(m_StatusBar);
}
void KMainWindow::CreateBaseLayout()
{
	m_WorkspaceContainer = new wxSimplebook(this, KxID_NONE);
	m_WorkspaceContainer->SetBackgroundColour(GetBackgroundColour());
	m_WorkspaceContainer->SetImageList(const_cast<KxImageList*>(KGetImageList()));
	m_MainSizer->Add(m_WorkspaceContainer, 1, wxEXPAND);
}
WXLRESULT KMainWindow::MSWWindowProc(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam)
{
	if (msg == WM_COPYDATA)
	{
		const COPYDATASTRUCT* data = reinterpret_cast<const COPYDATASTRUCT*>(lParam);
		const wxString link(reinterpret_cast<const wchar_t*>(data->lpData), data->cbData);
		KDownloadManager::GetInstance()->QueueFromOutside(link);
	}
	return KxFrame::MSWWindowProc(msg, wParam, lParam);
}

bool KMainWindow::Create(wxWindow* parent,
						 wxWindowID id,
						 const wxString& caption,
						 const wxPoint& pos,
						 const wxSize& size,
						 long style
)
{
	if (KxFrame::Create(parent, id, caption, pos, size, style))
	{
		SetWindowUserData(GetUniqueID());
		SetDefaultBackgroundColor();
		SetIcons(wxICON(IDS_ICON_APP));
		SetMinSize(size);
		
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
		SetSizer(m_MainSizer);

		CreateToolBar();
		CreateStatusBar();
		CreateBaseLayout();
		CreateMainWorkspaces();
		CreateMainMenu();

		KEvent::Bind(KEVT_VFS_TOGGLED, &KMainWindow::OnVFSToggled, this);
		KEvent::Bind(KEVT_VFS_TOGGLED, &KMainWindow::OnPluggableManagersMenuVFSToggled, this);

		KProgramOptionSerializer::LoadWindowSize(this, m_WindowOptions);
		return true;
	}
	return false;
}

void KMainWindow::CreatePluggableManagersWorkspaces(KWorkspace* parentWorkspace)
{
	KWorkspace* firstSubWorkspace = NULL;

	for (KManager* manager: KPluggableManager::GetInstances())
	{
		if (KPluggableManager* pluggableManager = manager->ToPluggableManager())
		{
			KWorkspace* workspace = pluggableManager->CreateWorkspace(this);
			if (workspace)
			{
				if (parentWorkspace && workspace->IsSubWorkspace())
				{
					if (!firstSubWorkspace)
					{
						firstSubWorkspace = workspace;
					}
					
					if (parentWorkspace->MakeSubWorkspace(workspace))
					{
						parentWorkspace->AddSubWorkspace(workspace);
						continue;
					}
				}

				AddWorkspace(workspace);
				KxMenuItem* item = workspace->CreateItemInManagersMenu();
				item->SetClientData(manager);
			}
		}
	}

	if (firstSubWorkspace)
	{
		firstSubWorkspace->CreateNow();
	}
}
void KMainWindow::CreateMainWorkspaces()
{
	m_ManagersMenu = new KxMenu();

	// Add workspaces
	AddWorkspace(new KGameConfigWorkspace(this))->CreateNow();
	AddWorkspace(new KModWorkspace(this))->CreateNow();
	AddWorkspace(new KPackageCreatorWorkspace(this));
	AddWorkspace(new KPackageManagerWorkspace(this));
	AddWorkspace(new KProgramManagerWorkspace(this));
	CreatePluggableManagersWorkspaces(KModWorkspace::GetInstance());

	// Create toolbar button and assign menu to it
	m_ToolBar->AddSeparator();

	KxAuiToolBarItem* toolBarButton = CreateToolBarButton(m_ToolBar, KVAR_EXP(KVAR_GAME_NAME));
	wxImage gameIcon = KGameInstance::GetActive()->GetIcon().ConvertToImage();
	toolBarButton->SetBitmap(gameIcon.Rescale(m_ToolBar->GetToolBitmapSize().GetWidth(), m_ToolBar->GetToolBitmapSize().GetHeight(), wxIMAGE_QUALITY_HIGH));

	toolBarButton->AssignDropdownMenu(m_ManagersMenu);
	toolBarButton->SetOptionEnabled(KxAUI_TBITEM_OPTION_LCLICK_MENU);

	m_ToolBar->Realize();
	m_ToolBar->SetOverflowVisible(!m_ToolBar->IsItemsFits());
}
void KMainWindow::CreateMainMenu()
{
	m_MainMenu = new KxMenu();
	m_ToolBar_MainMenu->AssignDropdownMenu(m_MainMenu);

	m_MainMenu_Settings = m_MainMenu->Add(new KxMenuItem(T("MainMenu.Settings")));
	m_MainMenu_Settings->SetBitmap(KGetBitmap(KIMG_APPLICATION_TASK));
	m_MainMenu_Settings->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
	{
		KSettingsWindow(this).ShowModal();
	});

	m_MainMenu->AddSeparator();

	m_MainMenu_ChangeProfile = m_MainMenu->Add(new KxMenuItem(T("MainMenu.ChangeProfile")));
	m_MainMenu_ChangeProfile->Bind(KxEVT_MENU_SELECT, &KMainWindow::OnChangeProfile, this);
	m_MainMenu_ChangeProfile->Enable(false);

	m_MainMenu->AddSeparator();

	KProgramManager::GetInstance()->OnAddMenuItems(m_MainMenu);

	m_MainMenu->AddSeparator();

	// Add locations
	KxMenu* locationsMenu = new KxMenu();
	const KLocationsManagerConfig* locationsManager = KLocationsManagerConfig::GetInstance();
	for (const KLabeledValue& entry: locationsManager->GetLocations())
	{
		if (entry.GetValue().IsEmpty() && entry.GetLabel().IsEmpty())
		{
			locationsMenu->AddSeparator();
		}
		else
		{
			KxMenuItem* item = locationsMenu->Add(new KxMenuItem(entry.GetLabel()));
			item->SetBitmap(KGetBitmap(KIMG_FOLDER));
			item->Bind(KxEVT_MENU_SELECT, [locationsManager, entry](KxMenuEvent& event)
			{
				locationsManager->OpenLocation(entry);
			});
		}
	}
	m_MainMenu->AppendSubMenu(locationsMenu, T("MainMenu.OpenLocation"))->SetBitmap(KGetBitmap(KIMG_FOLDER_OPEN));
	m_MainMenu->AddSeparator();

	KxMenuItem* pAboutItem = m_MainMenu->Add(new KxMenuItem(T("MainMenu.About")));
	pAboutItem->SetBitmap(KGetBitmap(KIMG_INFORMATION_FRAME));
	pAboutItem->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
	{
		KAboutDialog(this).ShowModal();
	});
}

void KMainWindow::OnQSMButton(KxAuiToolBarEvent& event)
{
	KWorkspace* current = GetCurrentWorkspace();
	if (current && current->HasQuickSettingsMenu())
	{
		DWORD alignment = TPM_RIGHTALIGN|TPM_TOPALIGN;
		wxPoint tPos = m_QuickToolBar_QuickSettingsMenu->GetRect().GetRightBottom() + wxPoint(1, 2);
		current->GetQuickSettingsMenu()->Show(m_QuickToolBar, tPos, alignment);
	}
}
void KMainWindow::OnWindowClose(wxCloseEvent& event)
{
	bool skip = false;

	if (event.CanVeto())
	{
		if (KModManager::Get().IsVFSMounted())
		{
			KxTaskDialog dialog(this, KxID_NONE, T("VFS.AskUnmountOnExit"), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_QUESTION);
			if (dialog.ShowModal() != KxID_YES)
			{
				event.Veto();
				return;
			}
		}

		bool veto = false;
		auto AskForSave = [&event, &veto](KWorkspace* workspace)
		{
			if (workspace && workspace->IsWorkspaceCreated())
			{
				KWorkspaceController* controller = workspace->GetWorkspaceController();
				if (controller && controller->AskForSave(event.CanVeto()) != KxID_OK)
				{
					veto = true;
					return false;
				}
			}
			return true;
		};

		KWorkspace* current = GetCurrentWorkspace();
		if (AskForSave(current))
		{
			for (const auto& v: m_WorkspaceInstances)
			{
				if (v.second != current)
				{
					if (!AskForSave(v.second))
					{
						break;
					}
				}
			}
		}

		if (veto)
		{
			event.Veto();
		}
		else
		{
			skip = true;
		}
	}
	else
	{
		skip = true;
	}

	if (skip)
	{
		event.Skip();
		KProgramOptionSerializer::SaveWindowSize(this, m_WindowOptions);
		KDownloadManager::GetInstance()->OnShutdown();
	}
}
void KMainWindow::OnChangeProfile(KxMenuEvent& event)
{
	KWorkspaceController* controller = GetCurrentWorkspace()->GetWorkspaceController();
	if (controller && controller->AskForSave() == KxID_CANCEL)
	{
		return;
	}

	if (KApp::Get().ShowChageInstanceDialog())
	{
		Close(true);
	}
}

void KMainWindow::OnVFSToggled(KVFSEvent& event)
{
	m_MainMenu_ChangeProfile->Enable(!event.IsActivated());

	// StatusBar
	if (event.IsActivated())
	{
		m_StatusBar->SetStatusText(T("VFS.Status.Active"));
		m_StatusBar->SetStatusImage(KIMG_TICK_CIRCLE_FRAME, 0);
	}
	else
	{
		m_StatusBar->SetStatusText(T("VFS.Status.Inactive"));
		m_StatusBar->SetStatusImage(KIMG_INFORMATION_FRAME_EMPTY, 0);
	}
	KThemeManager::Get().ProcessWindow(m_StatusBar, event.IsActivated());
}
void KMainWindow::OnPluggableManagersMenuVFSToggled(KVFSEvent& event)
{
	for (wxMenuItem* item: m_ManagersMenu->GetMenuItems())
	{
		KPluggableManager* manager = static_cast<KPluggableManager*>(static_cast<KxMenuItem*>(item)->GetClientData());
		if (manager)
		{
			//item->Enable(event.IsActivated());
		}
	}
}

KMainWindow::KMainWindow()
	:m_WindowOptions("KMainWindow", wxEmptyString)
{
	if (Create(NULL, KxID_NONE, KApp::Get().GetAppDisplayName(), wxDefaultPosition, wxSize(850, 600), DefaultStyle))
	{
		Bind(wxEVT_CLOSE_WINDOW, &KMainWindow::OnWindowClose, this);

		// Update status bar
		KVFSEvent event(false);
		OnVFSToggled(event);
	}
}
KMainWindow::~KMainWindow()
{
}

bool KMainWindow::SwitchWorkspaceHelper(KWorkspace* nextWorkspace, KWorkspace* prevWorkspace)
{
	wxLogInfo("%s: switching from %s to %s", __FUNCTION__, prevWorkspace ? prevWorkspace->GetID() : "NULL", nextWorkspace ? nextWorkspace->GetID() : "NULL");

	if (prevWorkspace && !prevWorkspace->OnCloseWorkspaceInternal())
	{
		wxLogInfo("%s: %s refused to close", __FUNCTION__, prevWorkspace->GetID());
		return false;
	}

	if (nextWorkspace && !nextWorkspace->IsWorkspaceCreated() && !nextWorkspace->OnCreateWorkspaceInternal())
	{
		wxLogInfo("%s: can not create %s workspace", __FUNCTION__, nextWorkspace->GetID());
		return false;
	}

	if (nextWorkspace && nextWorkspace->OnOpenWorkspaceInternal())
	{
		wxLogInfo("%s: %s opened. Process switching", __FUNCTION__, nextWorkspace->GetID());

		m_HasCurrentWorkspace = true;
		ProcessSwitchWorkspace(nextWorkspace, prevWorkspace);
		m_WorkspaceContainer->ChangeSelection((size_t)nextWorkspace->GetWorkspaceIndex());
		
		return true;
	}

	wxLogInfo("%s: %s refused to open", __FUNCTION__, nextWorkspace->GetID());
	return false;
}
void KMainWindow::ProcessSwitchWorkspace(KWorkspace* nextWorkspace, KWorkspace* prevWorkspace)
{
	wxLogInfo("%s: processing switch", __FUNCTION__);

	if (nextWorkspace)
	{
		m_QuickToolBar_QuickSettingsMenu->SetEnabled(nextWorkspace->HasQuickSettingsMenu());
		m_QuickToolBar_Help->SetEnabled(nextWorkspace->HasHelpEntry());

		wxWindowUpdateLocker lock(m_StatusBar);
		for (int i = 0; i < m_StatusBar->GetFieldsCount() - 1; i++)
		{
			ClearStatus(i);
		}
	}
}
KWorkspace* KMainWindow::DoAddWorkspace(KWorkspace* workspace)
{
	KThemeManager::Get().ProcessWindow(workspace);
	m_WorkspaceInstances.insert(std::make_pair(workspace->GetID(), workspace));
	m_WorkspaceContainer->AddPage(workspace, workspace->GetName(), false, workspace->GetImageID());

	return workspace;
}

KWorkspace* KMainWindow::GetWorkspace(const wxString& id) const
{
	wxLogInfo("Attempt to convert workspace ID (%s) to workspace instance", id);
	if (m_WorkspaceInstances.count(id))
	{
		return m_WorkspaceInstances.at(id);
	}
	return NULL;
}
KWorkspace* KMainWindow::GetCurrentWorkspace() const
{
	wxLogInfo("Attempt to get current workspace");

	wxWindow* window = m_WorkspaceContainer->GetCurrentPage();
	if (window && m_HasCurrentWorkspace)
	{
		return static_cast<KWorkspace*>(window);
	}
	return NULL;
}
KWorkspace* KMainWindow::GetFirstWorkspace() const
{
	wxLogInfo("Trying to get first workspace");

	if (!m_WorkspaceInstances.empty())
	{
		return (*m_WorkspaceInstances.begin()).second;
	}
	return NULL;
}
bool KMainWindow::SwitchWorkspace(KWorkspace* nextWorkspace)
{
	wxLogInfo("Attempt to switch workspace to %s", nextWorkspace ? nextWorkspace->GetID() : "NULL");

	if (nextWorkspace && !nextWorkspace->IsSubWorkspace())
	{
		KWorkspace* current = GetCurrentWorkspace();
		if (current != nextWorkspace)
		{
			return SwitchWorkspaceHelper(nextWorkspace, current);
		}
		else
		{
			return true;
		}
	}
	return false;
}
bool KMainWindow::SwitchWorkspace(const wxString& id)
{
	return SwitchWorkspace(GetWorkspace(id));
}

void KMainWindow::ClearStatus(int index)
{
	wxWindowUpdateLocker lock(m_StatusBar);

	m_StatusBar->SetStatusText(wxEmptyString, index + 1);
	m_StatusBar->SetStatusImage(KIMG_NONE, index + 1);
}
void KMainWindow::SetStatus(const wxString& label, int index, KImageEnum image)
{
	wxWindowUpdateLocker lock(m_StatusBar);

	m_StatusBar->SetStatusText(label, index + 1);
	m_StatusBar->SetStatusImage(image, index + 1);
}
void KMainWindow::SetStatusProgress(int current)
{
}
void KMainWindow::SetStatusProgress(int64_t current, int64_t total)
{
}
