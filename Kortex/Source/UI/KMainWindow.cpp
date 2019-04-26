#include "stdafx.h"
#include "KMainWindow.h"
#include <Kortex/Application.hpp>
#include <Kortex/Notification.hpp>
#include <Kortex/Events.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/ProgramManager.hpp>
#include <Kortex/DownloadManager.hpp>
#include <Kortex/GameInstance.hpp>
#include "Application/About/Dialog.h"
#include "Application/Settings/Window.h"
#include "KWorkspace.h"
#include "KWorkspaceController.h"
#include "Utility/KAux.h"
#include "Utility/Log.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxProcess.h>
#include <KxFramework/KxShell.h>

#include "PackageManager/KPackageManagerWorkspace.h"
#include "PackageCreator/KPackageCreatorWorkspace.h"

using namespace Kortex;

KxAuiToolBarItem* KMainWindow::CreateToolBarButton(KxAuiToolBar* toolBar, const wxString& label, KImageEnum imageID, wxItemKind kind, int index)
{
	wxBitmap bitmap = wxNullBitmap;
	if (imageID != KIMG_NONE)
	{
		bitmap = KGetBitmap(imageID);
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
		IThemeManager::GetActive().ProcessWindow(toolBar);

		m_ToolBarSizer->Add(toolBar, proportion, wxEXPAND);
		return toolBar;
	};

	/* Main ToolBar */
	m_ToolBar = NewToolBar(1, true);
	{
		m_ToolBar_MainMenu = m_ToolBar->AddTool(wxString::Format("%8s%s%8s", "", "Kortex", ""), KGetBitmap(KIMG_APPLICATION_LOGO_SMALL), wxITEM_DROPDOWN);
		m_ToolBar_MainMenu->SetOptionEnabled(KxAUI_TBITEM_OPTION_LCLICK_MENU);
		m_ToolBar_MainMenu->Bind(KxEVT_AUI_TOOLBAR_CLICK, [this](KxAuiToolBarEvent& evnt)
		{
			KxMenu menu;
			CreateMainMenu(menu);

			DWORD alignment = 0;
			wxPoint pos = m_ToolBar_MainMenu->GetDropdownMenuPosition(&alignment);
			menu.Show(m_ToolBar, pos, alignment);
		});

		m_ToolBar_InsertionIndex = m_ToolBar->AddSeparator()->GetIndex();
	}
	m_ToolBar->Realize();

	/* Quick ToolBar */
	m_QuickToolBar = NewToolBar(0, true);
	{
		AddToolBarButton<Kortex::INetworkManager>(m_QuickToolBar, KIMG_APPLICATION_LOGO_SMALL);
		m_QuickToolBar->AddSeparator();
		AddToolBarButton<Kortex::INotificationCenter>(m_QuickToolBar, KIMG_BELL);

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
		Kortex::IDownloadManager::GetInstance()->QueueFromOutside(link);
	}
	return KxFrame::MSWWindowProc(msg, wParam, lParam);
}

void KMainWindow::CreatePluggableManagersWorkspaces(KWorkspace* parentWorkspace)
{
	KWorkspace* firstSubWorkspace = nullptr;
	for (IModule* module: IModule::GetInstances())
	{
		for (IManager* manager: module->GetManagers())
		{
			if (IPluggableManager* pluggableManager = manager->ToPluggableManager())
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
	//AddWorkspace(new KGameConfigWorkspace(this))->CreateNow();
	AddWorkspace(new Kortex::ModManager::Workspace(this))->CreateNow();
	AddWorkspace(new KPackageCreatorWorkspace(this));
	AddWorkspace(new KPackageManagerWorkspace(this));
	CreatePluggableManagersWorkspaces(Kortex::ModManager::Workspace::GetInstance());

	// Create toolbar button and assign menu to it
	m_ToolBar->AddSeparator();

	KxAuiToolBarItem* toolBarButton = CreateToolBarButton(m_ToolBar, ITranslator::GetVariable(Variables::KVAR_GAME_NAME));
	wxImage gameIcon = IGameInstance::GetActive()->GetIcon().ConvertToImage();
	toolBarButton->SetBitmap(gameIcon.Rescale(m_ToolBar->GetToolBitmapSize().GetWidth(), m_ToolBar->GetToolBitmapSize().GetHeight(), wxIMAGE_QUALITY_HIGH));

	toolBarButton->AssignDropdownMenu(m_ManagersMenu);
	toolBarButton->SetOptionEnabled(KxAUI_TBITEM_OPTION_LCLICK_MENU);

	m_ToolBar->Realize();
	m_ToolBar->SetOverflowVisible(!m_ToolBar->IsItemsFits());
}
void KMainWindow::CreateMainMenu(KxMenu& mainMenu)
{
	{
		KxMenuItem* item = mainMenu.Add(new KxMenuItem(KTr("MainMenu.Settings")));
		item->SetBitmap(KGetBitmap(KIMG_APPLICATION_TASK));
		item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
		{
			Application::Settings::Window(this).ShowModal();
		});
	}
	mainMenu.AddSeparator();
	{
		KxMenuItem* item = mainMenu.Add(new KxMenuItem(KTr("MainMenu.ChangeInstance")));
		item->Bind(KxEVT_MENU_SELECT, &KMainWindow::OnChangeInstance, this);
		item->Enable(!Kortex::IModManager::GetInstance()->GetFileSystem().IsEnabled());
	}
	mainMenu.AddSeparator();

	// Add programs
	size_t count = mainMenu.GetMenuItemCount();
	Kortex::IProgramManager::GetInstance()->OnAddMainMenuItems(mainMenu);
	if (count != mainMenu.GetMenuItemCount())
	{
		mainMenu.AddSeparator();
	}

	// Add locations
	AddLocationsMenu(mainMenu);

	// Add about
	{
		KxMenuItem* item = mainMenu.Add(new KxMenuItem(KTr("MainMenu.About")));
		item->SetBitmap(KGetBitmap(KIMG_INFORMATION_FRAME));
		item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
		{
			Kortex::Application::AboutDialog(this).ShowModal();
		});
	}
}
void KMainWindow::AddLocationsMenu(KxMenu& mainMenu)
{
	// Set predefined locations
	if (m_Locations.empty())
	{
		using Variables::WrapAsInline;

		m_Locations.emplace_back(WrapAsInline(Variables::KVAR_APP_SETTINGS_DIR), KTr("OpenLocation.AppSettings"));
		m_Locations.emplace_back(WrapAsInline(Variables::KVAR_ACTUAL_GAME_DIR), KTr("OpenLocation.GameRoot"));
		m_Locations.emplace_back(WrapAsInline(Variables::KVAR_VIRTUAL_GAME_DIR), KTr("OpenLocation.VirtualGameRoot"));
		m_Locations.emplace_back(WrapAsInline(Variables::KVAR_ACTUAL_CONFIG_DIR), KTr("OpenLocation.ConfigRootTarget"));
		m_Locations.emplace_back(WrapAsInline(Variables::KVAR_CONFIG_DIR), KTr("OpenLocation.VirtualConfigRoot"));
		m_Locations.emplace_back(WrapAsInline(Variables::KVAR_OVERWRITES_DIR), KTr("OpenLocation.WriteTargetRoot"));
		m_Locations.emplace_back(WrapAsInline(Variables::KVAR_INSTANCE_DIR), KTr("OpenLocation.CurrentProfileRoot"));
		m_Locations.emplace_back(WrapAsInline(Variables::KVAR_INSTANCES_DIR), KTr("OpenLocation.ProfilesRoot"));
		m_Locations.emplace_back(WrapAsInline(Variables::KVAR_MODS_DIR), KTr("OpenLocation.ModsRoot"));
		m_Locations.emplace_back(WrapAsInline(Variables::KVAR_SAVES_DIR), KTr("OpenLocation.Saves"));

		// TODO: make main window a manager to allow it load instance config
		#if 0
		if (node.HasChildren())
		{
			// This will allow to insert a separator in locations menu
			m_Locations.emplace_back(KLabeledValue(wxEmptyString, wxEmptyString));

			// Load profile locations
			for (KxXMLNode entryNode = node.GetFirstChildElement("Entry"); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
			{
				KLabeledValue& value = m_Locations.emplace_back(KLabeledValue(KVarExp(entryNode.GetValue()), KVarExp(entryNode.GetAttribute("Label"))));
				if (value.GetValue().IsEmpty())
				{
					m_Locations.pop_back();
				}
			}
		}
		#endif
	}

	KxMenu* locationsMenu = new KxMenu();
	for (const KLabeledValue& entry: m_Locations)
	{
		if (!entry.HasLabel() && !entry.HasValue())
		{
			locationsMenu->AddSeparator();
		}
		else
		{
			KxMenuItem* item = locationsMenu->Add(new KxMenuItem(KVarExp(entry.GetLabel())));
			item->SetBitmap(KGetBitmap(KIMG_FOLDER));
			item->Bind(KxEVT_MENU_SELECT, [this, &entry](KxMenuEvent& event)
			{
				// Create the folder, shouldn't be harmful.
				KxFile folder(KVarExp(entry.GetValue()));
				folder.CreateFolder();

				return KxShell::Execute(this, folder.GetFullPath(), wxS("open"));
			});
		}
	}
	mainMenu.Add(locationsMenu, KTr("MainMenu.OpenLocation"))->SetBitmap(KGetBitmap(KIMG_FOLDER_OPEN));
	mainMenu.AddSeparator();
}

void KMainWindow::OnQSMButton(KxAuiToolBarEvent& event)
{
	KWorkspace* current = GetCurrentWorkspace();
	if (current && current->HasQuickSettingsMenu())
	{
		DWORD alignment = TPM_RIGHTALIGN|TPM_TOPALIGN;
		wxPoint pos = m_QuickToolBar_QuickSettingsMenu->GetRect().GetRightBottom() + wxPoint(1, 2);
		current->GetQuickSettingsMenu()->Show(m_QuickToolBar, pos, alignment);
	}
}
void KMainWindow::OnWindowClose(wxCloseEvent& event)
{
	bool skip = false;

	if (event.CanVeto())
	{
		if (Kortex::IModManager::GetInstance()->GetFileSystem().IsEnabled())
		{
			KxTaskDialog dialog(this, KxID_NONE, KTr("VFS.AskUnmountOnExit"), wxEmptyString, KxBTN_YES|KxBTN_NO, KxICON_QUESTION);
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
		GetAInstanceOption().SaveWindowLayout(this);
	}
}
void KMainWindow::OnChangeInstance(KxMenuEvent& event)
{
	KWorkspaceController* controller = GetCurrentWorkspace()->GetWorkspaceController();
	if (controller && controller->AskForSave() == KxID_CANCEL)
	{
		return;
	}

	if (IApplication::GetInstance()->OpenInstanceSelectionDialog())
	{
		Close(true);
	}
}

void KMainWindow::OnVFSToggled(VFSEvent& event)
{
	if (event.IsActivated())
	{
		m_StatusBar->SetStatusText(KTr("VFS.Status.Active"));
		m_StatusBar->SetStatusImage(KIMG_TICK_CIRCLE_FRAME, 0);
	}
	else
	{
		m_StatusBar->SetStatusText(KTr("VFS.Status.Inactive"));
		m_StatusBar->SetStatusImage(KIMG_INFORMATION_FRAME_EMPTY, 0);
	}
	IThemeManager::GetActive().ProcessWindow(m_StatusBar, event.IsActivated());
}
void KMainWindow::OnPluggableManagersMenuVFSToggled(VFSEvent& event)
{
	for (wxMenuItem* item: m_ManagersMenu->GetMenuItems())
	{
		IPluggableManager* manager = static_cast<IPluggableManager*>(static_cast<KxMenuItem*>(item)->GetClientData());
		if (manager)
		{
			//item->Enable(event.IsActivated());
		}
	}
}

bool KMainWindow::Create(wxWindow* parent)
{
	const wxSize size(850, 600);
	if (KxFrame::Create(parent, KxID_NONE, IApplication::GetInstance()->GetName(), wxDefaultPosition, size, KMainWindow::DefaultStyle))
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

		Bind(wxEVT_CLOSE_WINDOW, &KMainWindow::OnWindowClose, this);
		IEvent::Bind(Events::MainVFSToggled, &KMainWindow::OnVFSToggled, this);
		IEvent::Bind(Events::MainVFSToggled, &KMainWindow::OnPluggableManagersMenuVFSToggled, this);

		GetAInstanceOption().LoadWindowLayout(this);

		// Update status bar
		VFSEvent event(Events::MainVFSToggled, false);
		OnVFSToggled(event);
		return true;
	}
	return false;
}

bool KMainWindow::SwitchWorkspaceHelper(KWorkspace* nextWorkspace, KWorkspace* prevWorkspace)
{
	Utility::Log::LogInfo("%1: switching from %2 to %3", __FUNCTION__, prevWorkspace ? prevWorkspace->GetID() : "nullptr", nextWorkspace ? nextWorkspace->GetID() : "nullptr");

	if (prevWorkspace && !prevWorkspace->OnCloseWorkspaceInternal())
	{
		Utility::Log::LogInfo("%1: %2 refused to close", __FUNCTION__, prevWorkspace->GetID());
		return false;
	}

	if (nextWorkspace && !nextWorkspace->IsWorkspaceCreated() && !nextWorkspace->OnCreateWorkspaceInternal())
	{
		Utility::Log::LogInfo("%1: can not create %2 workspace", __FUNCTION__, nextWorkspace->GetID());
		return false;
	}

	if (nextWorkspace && nextWorkspace->OnOpenWorkspaceInternal())
	{
		Utility::Log::LogInfo("%1: %2 opened. Process switching", __FUNCTION__, nextWorkspace->GetID());

		m_HasCurrentWorkspace = true;
		ProcessSwitchWorkspace(nextWorkspace, prevWorkspace);
		m_WorkspaceContainer->ChangeSelection((size_t)nextWorkspace->GetWorkspaceIndex());
		
		return true;
	}

	Utility::Log::LogInfo("%1: %2 refused to open", __FUNCTION__, nextWorkspace->GetID());
	return false;
}
void KMainWindow::ProcessSwitchWorkspace(KWorkspace* nextWorkspace, KWorkspace* prevWorkspace)
{
	Utility::Log::LogInfo("%1: processing switch", __FUNCTION__);

	if (nextWorkspace)
	{
		m_QuickToolBar_QuickSettingsMenu->SetEnabled(nextWorkspace->HasQuickSettingsMenu());
		m_QuickToolBar_Help->SetEnabled(false);

		wxWindowUpdateLocker lock(m_StatusBar);
		for (int i = 0; i < m_StatusBar->GetFieldsCount() - 1; i++)
		{
			ClearStatus(i);
		}
	}
}
KWorkspace* KMainWindow::DoAddWorkspace(KWorkspace* workspace)
{
	IThemeManager::GetActive().ProcessWindow(workspace);
	m_WorkspaceInstances.insert(std::make_pair(workspace->GetID(), workspace));
	m_WorkspaceContainer->AddPage(workspace, workspace->GetName(), false, workspace->GetImageID());

	return workspace;
}

KWorkspace* KMainWindow::GetWorkspace(const wxString& id) const
{
	Utility::Log::LogInfo("Attempt to convert workspace ID (%1) to workspace instance", id);
	if (m_WorkspaceInstances.count(id))
	{
		return m_WorkspaceInstances.at(id);
	}
	return nullptr;
}
KWorkspace* KMainWindow::GetCurrentWorkspace() const
{
	Utility::Log::LogInfo("Attempt to get current workspace");

	wxWindow* window = m_WorkspaceContainer->GetCurrentPage();
	if (window && m_HasCurrentWorkspace)
	{
		return static_cast<KWorkspace*>(window);
	}
	return nullptr;
}
KWorkspace* KMainWindow::GetFirstWorkspace() const
{
	Utility::Log::LogInfo("Trying to get first workspace");

	if (!m_WorkspaceInstances.empty())
	{
		return (*m_WorkspaceInstances.begin()).second;
	}
	return nullptr;
}
bool KMainWindow::SwitchWorkspace(KWorkspace* nextWorkspace)
{
	Utility::Log::LogInfo("Attempt to switch workspace to %1", nextWorkspace ? nextWorkspace->GetID() : "nullptr");

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
