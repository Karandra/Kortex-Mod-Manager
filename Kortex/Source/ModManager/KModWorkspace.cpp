#include "stdafx.h"
#include "KModWorkspace.h"
#include "KVirtualGameFolderWorkspace.h"
#include "KModManagerModel.h"
#include "KModManager.h"
#include "KModEntry.h"
#include "KModStatistics.h"
#include "GameInstance/KInstanceManagement.h"
#include "Profile/KProfileEditor.h"
#include "KModTagsSelector.h"
#include "KModSites.h"
#include "KModFilesExplorerDialog.h"
#include "KModCollisionViewerModel.h"
#include "KModManagerImport.h"
#include "UI/KNewModDialog.h"
#include "UI/KImageViewerDialog.h"
#include "UI/KTextEditorDialog.h"
#include "InstallWizard/KInstallWizardDialog.h"
#include "SaveManager/KSaveManagerWorkspace.h"
#include "PluginManager/KPluginManager.h"
#include "PluginManager/KPluginManagerWorkspace.h"
#include "ScreenshotsGallery/KScreenshotsGalleryManager.h"
#include "PackageCreator/KPackageCreatorWorkspace.h"
#include "ProgramManager/KProgramWorkspace.h"
#include "Network/KNetwork.h"
#include "KEvents.h"
#include "KThemeManager.h"
#include "KOperationWithProgress.h"
#include "KAux.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxSearchBox.h>
#include <KxFramework/KxParagraph.h>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxDualProgressDialog.h>
#include <KxFramework/KxFileOperationEvent.h>
#include <wx/colordlg.h>

enum DisplayModeMenuID
{
	Connector,
	Manager,
	ShowPriorityGroups,
	BoldPriorityGroupLabels,
	ShowNotInstalledMods,

	PriorityGroupLabelAlignment_Left,
	PriorityGroupLabelAlignment_Right,
	PriorityGroupLabelAlignment_Center,
};
enum ToolsMenuID
{
	TOOLS_ID_STATISTICS = KxID_HIGHEST + 1,
	TOOLS_ID_EXPORT_MOD_LIST,
};
enum ContextMenuID
{
	KMC_ID_START = KxID_HIGHEST,

	KMC_ID_MOD_OPEN_LOCATION,
	KMC_ID_MOD_CHANGE_LOCATION,
	KMC_ID_MOD_REVERT_LOCATION,
	KMC_ID_MOD_INSTALL,
	KMC_ID_MOD_UNINSTALL,
	KMC_ID_MOD_UNINSTALL_AND_ERASE,
	KMC_ID_MOD_IMAGE_SHOW,
	KMC_ID_MOD_IMAGE_ASSIGN,
	KMC_ID_MOD_EDIT_DESCRIPTION,
	KMC_ID_MOD_EDIT_TAGS,
	KMC_ID_MOD_EDIT_SITES,
	KMC_ID_MOD_CHANGE_ID,
	KMC_ID_MOD_EXPLORE_FILES,
	KMC_ID_MOD_SHOW_COLLISIONS,
	KMC_ID_MOD_PROPERTIES,

	KMC_ID_PACKAGE_OPEN,
	KMC_ID_PACKAGE_ASSIGN,
	KMC_ID_PACKAGE_OPEN_LOCATION,
	KMC_ID_PACKAGE_REMOVE,
	KMC_ID_PACKAGE_EXTRACT,
	KMC_ID_PACKAGE_IMPORT_PROJECT,
	KMC_ID_PACKAGE_CREATE_PROJECT,
	KMC_ID_PACKAGE_PROPERTIES,

	KMC_COLOR_ASSIGN,
	KMC_COLOR_RESET,

	KMC_ID_TAG_ENABLE_ALL,
	KMC_ID_TAG_DISABLE_ALL,
};

namespace
{
	template<class Functor> bool DoForAllSelectedItems(KModManagerModel* model, const Functor& func)
	{
		KxDataViewItem::Vector items;
		if (model->GetView()->GetSelections(items) != 0)
		{
			for (const KxDataViewItem& item: items)
			{
				KModEntry* entry = model->GetModEntry(item);
				if (entry)
				{
					if (!func(*entry))
					{
						break;
					}
				}
			}
			return true;
		}
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
KModWorkspace::KModWorkspace(KMainWindow* mainWindow)
	:KWorkspace(mainWindow), m_OptionsUI(this, "MainUI"), m_ModListViewOptions(this, "ModListView")
{
	CreateToolBarButton();
	m_MainSizer = new wxBoxSizer(wxVERTICAL);
}
KModWorkspace::~KModWorkspace()
{
	if (IsWorkspaceCreated())
	{
		KProgramOptionSerializer::SaveDataViewLayout(m_ViewModel->GetView(), m_ModListViewOptions);
		m_ModListViewOptions.SetAttribute("DisplayMode", m_ViewModel->GetDisplayMode());
		m_ModListViewOptions.SetAttribute("ShowPriorityGroups", m_ViewModel->ShouldShowPriorityGroups());
		m_ModListViewOptions.SetAttribute("ShowNotInstalledMods", m_ViewModel->ShouldShowNotInstalledMods());
		m_ModListViewOptions.SetAttribute("BoldPriorityGroupLabels", m_ViewModel->IsBoldPriorityGroupLabels());
		m_ModListViewOptions.SetAttribute("PriorityGroupLabelAlignment", (int)m_ViewModel->GetPriorityGroupLabelAlignment());
		m_ModListViewOptions.SetAttribute("ImageResizeMode", (int)m_ImageResizeMode);

		KProgramOptionSerializer::SaveSplitterLayout(m_SplitterLeftRight, m_OptionsUI);
	}
}
bool KModWorkspace::OnCreateWorkspace()
{
	m_SplitterLeftRight = new KxSplitterWindow(this, KxID_NONE);
	m_SplitterLeftRight->SetName("ModViewPaneSize");
	m_SplitterLeftRight->SetMinimumPaneSize(250);
	m_MainSizer->Add(m_SplitterLeftRight, 1, wxEXPAND);
	KThemeManager::Get().ProcessWindow(m_SplitterLeftRight, true);

	// Mods Panel
	m_ModsPaneSizer = new wxBoxSizer(wxVERTICAL);
	m_ModsPane = new KxPanel(m_SplitterLeftRight, KxID_NONE);
	m_ModsPane->SetSizer(m_ModsPaneSizer);
	m_ModsPane->SetBackgroundColour(GetBackgroundColour());

	CreateModsView();
	CreateToolBar();
	m_ModsPaneSizer->Add(m_ModsToolBar, 0, wxEXPAND);
	m_ModsPaneSizer->Add(m_ViewModel->GetView(), 1, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);

	CreateRightPane();
	CreateControls();

	ReloadView();
	UpdateModListContent();

	KEvent::Bind(KEVT_VFS_TOGGLED, &KModWorkspace::OnVFSToggled, this);

	CreateAsSubWorkspace<KVirtualGameFolderWorkspace>();
	CreateAsSubWorkspace<KProgramWorkspace>();
	return true;
}

void KModWorkspace::CreateToolBar()
{
	m_ModsToolBar = new KxAuiToolBar(m_ModsPane, KxID_NONE, wxAUI_TB_HORZ_TEXT|wxAUI_TB_PLAIN_BACKGROUND);
	m_ModsToolBar->SetBackgroundColour(GetBackgroundColour());
	m_ModsToolBar->SetMargins(0, 1, 0, 0);

	m_ToolBar_Profiles = new KxComboBox(m_ModsToolBar, KxID_NONE);
	m_ToolBar_Profiles->Bind(wxEVT_COMBOBOX, &KModWorkspace::OnSelectProfile, this);

	m_ModsToolBar->AddLabel(KTr("ModManager.Profile") + ':');
	m_ModsToolBar->AddControl(m_ToolBar_Profiles)->SetProportion(1);

	m_ToolBar_EditProfiles = KMainWindow::CreateToolBarButton(m_ModsToolBar, wxEmptyString, KIMG_GEAR);
	m_ToolBar_EditProfiles->SetShortHelp(KTr("ModManager.Profile.Configure"));
	m_ToolBar_EditProfiles->Bind(KxEVT_AUI_TOOLBAR_CLICK, &KModWorkspace::OnShowProfileEditor, this);
	m_ModsToolBar->AddSeparator();

	m_ToolBar_AddMod = KMainWindow::CreateToolBarButton(m_ModsToolBar, KTr(KxID_ADD), KIMG_PLUS_SMALL);
	
	m_ToolBar_ChangeDisplayMode = KMainWindow::CreateToolBarButton(m_ModsToolBar, KTr("ModManager.DisplayMode.Caption"), KIMG_PROJECTION_SCREEN);
	m_ToolBar_ChangeDisplayMode->Bind(KxEVT_AUI_TOOLBAR_CLICK, &KModWorkspace::OnDisplayModeMenu, this);
	
	m_ToolBar_Tools = KMainWindow::CreateToolBarButton(m_ModsToolBar, wxEmptyString, KIMG_WRENCH_SCREWDRIVER);
	m_ToolBar_Tools->SetShortHelp(KTr("ModManager.Tools"));
	m_ToolBar_Tools->Bind(KxEVT_AUI_TOOLBAR_CLICK, &KModWorkspace::OnToolsMenu, this);

	m_SearchBox = new KxSearchBox(m_ModsToolBar, KxID_NONE);
	m_SearchBox->SetName("KModWorkspace search box");
	m_SearchBox->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &KModWorkspace::OnModSerach, this);
	m_SearchBox->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &KModWorkspace::OnModSerach, this);

	KxMenu* searchMenu = new KxMenu();
	searchMenu->Bind(KxEVT_MENU_SELECT, &KModWorkspace::OnModSearchColumnsChanged, this);
	m_ViewModel->CreateSearchColumnsMenu(*searchMenu);
	m_SearchBox->SetMenu(searchMenu);

	m_ModsToolBar->AddControl(m_SearchBox);

	m_ModsToolBar->Realize();

	CreateDisplayModeMenu();
	CreateAddModMenu();
	CreateToolsMenu();
}
void KModWorkspace::CreateDisplayModeMenu()
{
	m_ToolBar_DisplayModeMenu = new KxMenu();
	m_ToolBar_ChangeDisplayMode->AssignDropdownMenu(m_ToolBar_DisplayModeMenu);
	m_ToolBar_ChangeDisplayMode->SetOptionEnabled(KxAUI_TBITEM_OPTION_LCLICK_MENU);

	{
		KxMenuItem* item = m_ToolBar_DisplayModeMenu->Add(new KxMenuItem(DisplayModeMenuID::Connector, KTr("ModManager.DisplayMode.Connector"), wxEmptyString, wxITEM_RADIO));
		item->SetBitmap(KGetBitmap(KIMG_PLUG_DISCONNECT));
		item->Check(m_ViewModel->GetDisplayMode() == KMM_TYPE_CONNECTOR);
	}
	{
		KxMenuItem* item = m_ToolBar_DisplayModeMenu->Add(new KxMenuItem(DisplayModeMenuID::Manager, KTr("ModManager.DisplayMode.Log"), wxEmptyString, wxITEM_RADIO));
		item->SetBitmap(KGetBitmap(KIMG_CATEGORY));
		item->Check(m_ViewModel->GetDisplayMode() == KMM_TYPE_MANAGER);
	}
	m_ToolBar_DisplayModeMenu->AddSeparator();

	{
		KxMenuItem* item = m_ToolBar_DisplayModeMenu->Add(new KxMenuItem(DisplayModeMenuID::ShowPriorityGroups, KTr("ModManager.DisplayMode.ShowPriorityGroups"), wxEmptyString, wxITEM_CHECK));
		item->SetBitmap(KGetBitmap(KIMG_FOLDERS));
		item->Check(m_ViewModel->ShouldShowPriorityGroups());
	}
	{
		KxMenuItem* item = m_ToolBar_DisplayModeMenu->Add(new KxMenuItem(DisplayModeMenuID::ShowNotInstalledMods, KTr("ModManager.DisplayMode.ShowNotInstalledMods"), wxEmptyString, wxITEM_CHECK));
		item->Check(m_ViewModel->ShouldShowNotInstalledMods());
	}
	{
		KxMenuItem* item = m_ToolBar_DisplayModeMenu->Add(new KxMenuItem(DisplayModeMenuID::BoldPriorityGroupLabels, KTr("ModManager.DisplayMode.BoldPriorityGroupLabels"), wxEmptyString, wxITEM_CHECK));
		item->Check(m_ViewModel->IsBoldPriorityGroupLabels());
	}
	m_ToolBar_DisplayModeMenu->AddSeparator();

	{
		KxMenu* alignmnetMenu = new KxMenu();
		m_ToolBar_DisplayModeMenu->Add(alignmnetMenu, KTr("ModManager.DisplayMode.PriorityGroupLabelsAlignment"));

		using PriorityGroupLabelAlignment = KModManagerModel::PriorityGroupLabelAlignment;
		auto AddOption = [this, alignmnetMenu](DisplayModeMenuID id, PriorityGroupLabelAlignment type, KxStandardID trId)
		{
			KxMenuItem* item = alignmnetMenu->Add(new KxMenuItem(id, KTr(trId), wxEmptyString, wxITEM_RADIO));
			item->Check(m_ViewModel->GetPriorityGroupLabelAlignment() == type);
			return item;
		};
		AddOption(DisplayModeMenuID::PriorityGroupLabelAlignment_Left, PriorityGroupLabelAlignment::Left, KxID_JUSTIFY_LEFT);
		AddOption(DisplayModeMenuID::PriorityGroupLabelAlignment_Center, PriorityGroupLabelAlignment::Center, KxID_JUSTIFY_CENTER);
		AddOption(DisplayModeMenuID::PriorityGroupLabelAlignment_Right, PriorityGroupLabelAlignment::Right, KxID_JUSTIFY_RIGHT);
	}
}
void KModWorkspace::CreateAddModMenu()
{
	m_ToolBar_AddModMenu = new KxMenu();
	m_ToolBar_AddMod->AssignDropdownMenu(m_ToolBar_AddModMenu);
	m_ToolBar_AddMod->SetOptionEnabled(KxAUI_TBITEM_OPTION_LCLICK_MENU);
	KxMenuItem* item = NULL;

	item = m_ToolBar_AddModMenu->Add(new KxMenuItem(KTr("ModManager.NewMod.NewEmptyMod")));
	item->SetBitmap(KGetBitmap(KIMG_FOLDER_PLUS));
	item->Bind(KxEVT_MENU_SELECT, &KModWorkspace::OnAddMod_Empty, this);

	item = m_ToolBar_AddModMenu->Add(new KxMenuItem(KTr("ModManager.NewMod.NewModFromFolder")));
	item->SetBitmap(KGetBitmap(KIMG_FOLDER_ARROW));
	item->Bind(KxEVT_MENU_SELECT, &KModWorkspace::OnAddMod_FromFolder, this);
	
	item = m_ToolBar_AddModMenu->Add(new KxMenuItem(KTr("ModManager.NewMod.InstallPackage")));
	item->SetBitmap(KGetBitmap(KIMG_BOX_SEARCH_RESULT));
	item->Bind(KxEVT_MENU_SELECT, &KModWorkspace::OnAddMod_InstallPackage, this);

	m_ToolBar_AddModMenu->AddSeparator();

	item = m_ToolBar_AddModMenu->Add(new KxMenuItem(KTrf("ModManager.NewMod.ImportFrom", "Mod Organizer")));
	item->SetBitmap(KGetBitmap(KIMG_MO2));
	item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
	{
		KModManagerImport::ShowImportDialog(KModManagerImport::ModOrganizer, this);
	});

	item = m_ToolBar_AddModMenu->Add(new KxMenuItem(KTrf("ModManager.NewMod.ImportFrom", "Nexus Mod Manager")));
	item->SetBitmap(KGetBitmap(KIMG_SITE_NEXUS));
	item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
	{
		KModManagerImport::ShowImportDialog(KModManagerImport::NexusModManager, this);
	});
}
void KModWorkspace::CreateToolsMenu()
{
	KxMenu* menu = new KxMenu();
	m_ToolBar_Tools->AssignDropdownMenu(menu);
	m_ToolBar_Tools->SetOptionEnabled(KxAUI_TBITEM_OPTION_LCLICK_MENU);
	KxMenuItem* item = NULL;

	item = menu->Add(new KxMenuItem(TOOLS_ID_STATISTICS, KTr("ModManager.Statistics")));
	item->SetBitmap(KGetBitmap(KIMG_CHART));

	item = menu->Add(new KxMenuItem(TOOLS_ID_EXPORT_MOD_LIST, KTr("Generic.Export")));
	item->SetBitmap(KGetBitmap(KIMG_DISK));
}

void KModWorkspace::CreateModsView()
{
	m_ViewModel = new KModManagerModel();
	m_ViewModel->ShowPriorityGroups(m_ModListViewOptions.GetAttributeBool("ShowPriorityGroups"));
	m_ViewModel->ShowNotInstalledMods(m_ModListViewOptions.GetAttributeBool("ShowNotInstalledMods"));
	m_ViewModel->SetBoldPriorityGroupLabels(m_ModListViewOptions.GetAttributeBool("BoldPriorityGroupLabels"));
	m_ViewModel->SetPriorityGroupLabelAlignment((KModManagerModel::PriorityGroupLabelAlignment)m_ModListViewOptions.GetAttributeInt("PriorityGroupLabelAlignment"));
	
	m_ViewModel->Create(m_ModsPane);
	m_ViewModel->SetDataVector(KModManager::GetInstance()->GetEntries());
	m_ViewModel->SetDisplayMode((KModManagerModelType)m_ModListViewOptions.GetAttributeInt("DisplayMode"));
}
void KModWorkspace::CreateControls()
{
	wxBoxSizer* controlsSizer = new wxBoxSizer(wxHORIZONTAL);
	m_MainSizer->Add(controlsSizer, 0, wxEXPAND|wxALIGN_RIGHT|wxTOP, KLC_VERTICAL_SPACING);

	m_ActivateButton = new KxButton(this, KxID_NONE, KTr("ModManager.VFS.Activate"));
	m_ActivateButton->SetBitmap(KGetBitmap(KIMG_TICK_CIRCLE_FRAME_EMPTY));
	m_ActivateButton->Bind(wxEVT_BUTTON, &KModWorkspace::OnMountButton, this);

	controlsSizer->Add(m_ActivateButton);
}
void KModWorkspace::CreateRightPane()
{
	m_PaneRight_Tabs = new KxAuiNotebook(m_SplitterLeftRight, KxID_NONE);
	m_PaneRight_Tabs->SetImageList(KGetImageList());

	m_PaneRight_Tabs->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGING, &KModWorkspace::OnSubWorkspaceOpening, this);
	m_PaneRight_Tabs->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &KModWorkspace::OnSubWorkspaceOpened, this);
}

bool KModWorkspace::OnOpenWorkspace()
{
	if (IsFirstTimeOpen())
	{
		m_SplitterLeftRight->SplitVertically(m_ModsPane, m_PaneRight_Tabs);
		if (m_PaneRight_Tabs->GetPageCount() == 0)
		{
			m_SplitterLeftRight->Unsplit(m_PaneRight_Tabs);
		}

		KProgramOptionSerializer::LoadSplitterLayout(m_SplitterLeftRight, m_OptionsUI);
		KProgramOptionSerializer::LoadDataViewLayout(m_ViewModel->GetView(), m_ModListViewOptions);

		m_ImageResizeMode = (ImageResizeMode)m_ModListViewOptions.GetAttributeInt("ImageResizeMode", (int)m_ImageResizeMode);
		m_ViewModel->UpdateRowHeight();
	}
	return true;
}
bool KModWorkspace::OnCloseWorkspace()
{
	GetMainWindow()->ClearStatus();
	return true;
}
void KModWorkspace::OnReloadWorkspace()
{
	ClearControls();
	
	m_ViewModel->RefreshItems();
	ProcessSelection();
	UpdateModListContent();
}

wxString KModWorkspace::GetID() const
{
	return "KModWorkspace";
}
wxString KModWorkspace::GetName() const
{
	return KTr("ModManager.Name");
}

bool KModWorkspace::AddSubWorkspace(KWorkspace* workspace)
{
	m_PaneRight_Tabs->InsertPage(workspace->GetTabIndex(), workspace, workspace->GetNameShort(), workspace->GetTabIndex() == 0, workspace->GetImageID());
	return true;
}
void KModWorkspace::OnSubWorkspaceOpening(wxAuiNotebookEvent& event)
{
	event.Skip();
	KWorkspace* oldWorkspace = static_cast<KWorkspace*>(m_PaneRight_Tabs->GetPage(event.GetOldSelection()));
	KWorkspace* workspace = static_cast<KWorkspace*>(m_PaneRight_Tabs->GetPage(event.GetSelection()));

	if (oldWorkspace != workspace)
	{
		if (oldWorkspace && !oldWorkspace->OnCloseWorkspaceInternal())
		{
			event.Veto();
			return;
		}

		if (workspace && workspace->CreateNow() && workspace->OnOpenWorkspaceInternal())
		{
			event.Allow();
		}
	}
}
void KModWorkspace::OnSubWorkspaceOpened(wxAuiNotebookEvent& event)
{
	event.Skip();

	KWorkspace* workspace = static_cast<KWorkspace*>(m_PaneRight_Tabs->GetPage(event.GetSelection()));
	if (workspace && workspace->IsWorkspaceCreated())
	{
		event.Allow();
	}
}

void KModWorkspace::OnMountButton(wxCommandEvent& event)
{
	KModManager* manager = KModManager::GetInstance();
	if (manager->IsVFSMounted())
	{
		manager->UnMountVFS();
	}
	else
	{
		manager->MountVFS();
	}
}
bool KModWorkspace::ShowChangeModIDDialog(KModEntry* entry)
{
	wxString newID;
	KxTextBoxDialog dialog(GetMainWindow(), KxID_NONE, KTr("ModManager.Menu.ChangeID"), wxDefaultPosition, wxDefaultSize, KxBTN_OK|KxBTN_CANCEL);
	dialog.SetValue(entry->GetID());
	dialog.Bind(KxEVT_STDDIALOG_BUTTON, [this, entry, &dialog, &newID](wxNotifyEvent& event)
	{
		if (event.GetId() == KxID_OK)
		{
			newID = dialog.GetValue();
			if (newID.IsEmpty())
			{
				KxTaskDialog(&dialog, KxID_NONE, KTr("InstallWizard.ChangeID.Invalid"), wxEmptyString, KxBTN_OK, KxICON_WARNING).ShowModal();
				event.Veto();
			}
			else if (const KModEntry* existingMod = KModManager::GetInstance()->FindModByID(newID))
			{
				if (existingMod != entry)
				{
					KxTaskDialog(&dialog, KxID_NONE, KTrf("InstallWizard.ChangeID.Used", existingMod->GetName()), wxEmptyString, KxBTN_OK, KxICON_WARNING).ShowModal();
					event.Veto();
				}
			}
		}
	});

	if (dialog.ShowModal() == KxID_OK && newID != entry->GetID())
	{
		return KModManager::GetInstance()->ChangeModID(entry, newID);
	}
	return false;
}

void KModWorkspace::ProcessSelectProfile(const wxString& newProfileID)
{
	if (!newProfileID.IsEmpty())
	{
		KGameInstance* instance = KGameInstance::GetActive();
		KProfile* profile = KGameInstance::GetActiveProfile();

		if (!KGameInstance::IsActiveProfileID(newProfileID))
		{
			profile->SyncWithCurrentState();
			profile->Save();

			KProfile* newProfile = instance->GetProfile(newProfileID);
			if (newProfile)
			{
				instance->ChangeProfileTo(*newProfile);
			}
		}
	}
}
void KModWorkspace::OnSelectProfile(wxCommandEvent& event)
{
	ProcessSelectProfile(m_ToolBar_Profiles->GetString(event.GetSelection()));
	m_ViewModel->GetView()->SetFocus();
}
void KModWorkspace::OnShowProfileEditor(KxAuiToolBarEvent& event)
{
	// Save current
	KProfile* profile = KGameInstance::GetActiveProfile();
	profile->SyncWithCurrentState();
	profile->Save();

	KModListManagerEditorDialog dialog(this);
	dialog.ShowModal();
	if (dialog.IsModified())
	{
		ProcessSelectProfile(dialog.GetNewProfile());
		UpdateModListContent();
	}
}

void KModWorkspace::OpenPackage(const wxString& path)
{
	KInstallWizardDialog* dialog = new KInstallWizardDialog(GetMainWindow(), path);
	dialog->Bind(KEVT_IW_DONE, [this](wxNotifyEvent& event)
	{
		ReloadWorkspace();
	});
}

void KModWorkspace::OnDisplayModeMenu(KxAuiToolBarEvent& event)
{
	wxWindowID id = m_ToolBar_ChangeDisplayMode->ShowDropdownMenu();
	if (id != KxID_NONE)
	{
		switch (id)
		{
			case DisplayModeMenuID::Connector:
			{
				m_ViewModel->SetDisplayMode(KMM_TYPE_CONNECTOR);
				m_ViewModel->RefreshItems();
				ProcessSelection();
				break;
			}
			case DisplayModeMenuID::Manager:
			{
				m_ViewModel->SetDisplayMode(KMM_TYPE_MANAGER);
				m_ViewModel->RefreshItems();
				ProcessSelection();
				break;
			}
			case DisplayModeMenuID::ShowPriorityGroups:
			{
				m_ViewModel->ShowPriorityGroups(!m_ViewModel->ShouldShowPriorityGroups());
				m_ViewModel->RefreshItems();
				ProcessSelection();
				break;
			}
			case DisplayModeMenuID::ShowNotInstalledMods:
			{
				m_ViewModel->ShowNotInstalledMods(!m_ViewModel->ShouldShowNotInstalledMods());
				m_ViewModel->RefreshItems();
				ProcessSelection();
				break;
			}
			case DisplayModeMenuID::BoldPriorityGroupLabels:
			{
				m_ViewModel->SetBoldPriorityGroupLabels(!m_ViewModel->IsBoldPriorityGroupLabels());
				m_ViewModel->UpdateUI();
				break;
			}

			using PriorityGroupLabelAlignment = KModManagerModel::PriorityGroupLabelAlignment;
			case DisplayModeMenuID::PriorityGroupLabelAlignment_Left:
			{
				m_ViewModel->SetPriorityGroupLabelAlignment(PriorityGroupLabelAlignment::Left);
				m_ViewModel->UpdateUI();
				break;
			}
			case DisplayModeMenuID::PriorityGroupLabelAlignment_Right:
			{
				m_ViewModel->SetPriorityGroupLabelAlignment(PriorityGroupLabelAlignment::Right);
				m_ViewModel->UpdateUI();
				break;
			}
			case DisplayModeMenuID::PriorityGroupLabelAlignment_Center:
			{
				m_ViewModel->SetPriorityGroupLabelAlignment(PriorityGroupLabelAlignment::Center);
				m_ViewModel->UpdateUI();
				break;
			}
		};
	}
}
void KModWorkspace::OnToolsMenu(KxAuiToolBarEvent& event)
{
	wxWindowID id = m_ToolBar_Tools->ShowDropdownMenu();
	switch (id)
	{
		case TOOLS_ID_STATISTICS:
		{
			KModStatisticsDialog dialog(this);
			dialog.ShowModal();
			break;
		}
		case TOOLS_ID_EXPORT_MOD_LIST:
		{
			KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_SAVE);
			dialog.AddFilter("*.html", KTr("FileFilter.HTML"));
			dialog.SetDefaultExtension("html");
			dialog.SetFileName(KApp::Get().GetCurrentGameID() + " - " + KApp::Get().GetCurrentInstanceID());

			if (dialog.ShowModal() == KxID_OK)
			{
				KModManager::GetInstance()->ExportModList(dialog.GetResult());
			}
			break;
		}
	};
}

void KModWorkspace::OnVFSToggled(KVFSEvent& event)
{
	m_ToolBar_Profiles->Enable(!event.IsActivated());
	m_ToolBar_EditProfiles->SetEnabled(!event.IsActivated());

	wxWindowUpdateLocker lock(m_ActivateButton);
	if (event.IsActivated())
	{
		m_ActivateButton->SetLabel(KTr("ModManager.VFS.Deactivate"));
		m_ActivateButton->SetBitmap(KGetBitmap(KIMG_INFORMATION_FRAME_EMPTY));
	}
	else
	{
		m_ActivateButton->SetLabel(KTr("ModManager.VFS.Activate"));
		m_ActivateButton->SetBitmap(KGetBitmap(KIMG_TICK_CIRCLE_FRAME_EMPTY));
	}
	m_ViewModel->UpdateUI();
}
void KModWorkspace::OnAddMod_Empty(KxMenuEvent& event)
{
	KNewModDialog dialog(this);
	if (dialog.ShowModal() == KxID_OK)
	{
		KModEntry entry;
		entry.SetID(dialog.GetFolderName());
		entry.CreateAllFolders();
		entry.SetTime(KME_TIME_INSTALL, wxDateTime::Now());
		entry.Save();

		KModEvent(KEVT_MOD_INSTALLED, entry).Send();
	}
}
void KModWorkspace::OnAddMod_FromFolder(KxMenuEvent& event)
{
	KNewModDialogEx dialog(this);
	if (dialog.ShowModal() == KxID_OK)
	{
		KModEntry modEntry;
		modEntry.SetID(dialog.GetFolderName());
		modEntry.CreateAllFolders();
		modEntry.SetTime(KME_TIME_INSTALL, wxDateTime::Now());

		if (dialog.IsCreateAsLinkedMod())
		{
			modEntry.SetLinkedModLocation(dialog.GetFolderPath());
		}
		modEntry.Save();

		if (!dialog.IsCreateAsLinkedMod())
		{
			// Copy files
			wxString sourcePath = dialog.GetFolderPath();
			wxString destinationPath = modEntry.GetModFilesDir();
			auto operation = new KOperationWithProgressDialog<KxFileOperationEvent>(true, this);
			operation->OnRun([sourcePath, destinationPath](KOperationWithProgressBase* self)
			{
				KxEvtFile folder(sourcePath);
				self->LinkHandler(&folder, KxEVT_FILEOP_COPY_FOLDER);
				folder.CopyFolder(KxFile::NullFilter, destinationPath, true, true);
			});

			// If canceled, remove entire mod folder
			wxString modRoot = modEntry.GetRootDir();
			operation->OnCancel([modRoot](KOperationWithProgressBase* self)
			{
				KxFile(modRoot).RemoveFolderTree(true);
			});

			// Reload after task is completed (successfully or not)
			operation->OnEnd([this, name = modEntry.GetID().Clone()](KOperationWithProgressBase* self)
			{
				KModEvent(KEVT_MOD_INSTALLED, name).Send();
				ReloadWorkspace();
			});

			// Configure and run
			operation->SetDialogCaption(KTr("ModManager.NewMod.CopyDialogCaption"));
			operation->Run();
		}
		else
		{
			KModEvent(KEVT_MOD_INSTALLED, modEntry).Send();
		}
	}
}
void KModWorkspace::OnAddMod_InstallPackage(KxMenuEvent& event)
{
	KxFileBrowseDialog dialog(GetMainWindow(), KxID_NONE, KxFBD_OPEN);
	dialog.AddFilter("*.kmp;*.smi;*.7z;*.zip;*.fomod", KTr("FileFilter.AllSupportedFormats"));
	dialog.AddFilter("*.kmp", KTr("FileFilter.ModPackage"));
	dialog.AddFilter("*", KTr("FileFilter.AllFiles"));
	if (dialog.ShowModal() == KxID_OK)
	{
		OpenPackage(dialog.GetResult());
	}
}

void KModWorkspace::InstallMod(KModEntry* entry)
{
	new KInstallWizardDialog(this, entry->GetInstallPackageFile());
}
void KModWorkspace::UninstallMod(KModEntry* entry, bool eraseLog)
{
	if (entry)
	{
		KxTaskDialog dialog(GetMainWindow(), KxID_NONE, wxEmptyString, KTr("ModManager.RemoveMod.Message"), KxBTN_YES|KxBTN_NO, KxICON_WARNING);
		if (entry->IsInstalled())
		{
			dialog.SetCaption(eraseLog ? KTrf("ModManager.RemoveMod.CaptionUninstallAndErase", entry->GetName()) : KTrf("ModManager.RemoveMod.CaptionUninstall", entry->GetName()));
		}
		else
		{
			dialog.SetCaption(KTrf("ModManager.RemoveMod.CaptionErase", entry->GetName()));
			eraseLog = true;
		}

		if (dialog.ShowModal() == KxID_YES)
		{
			if (eraseLog)
			{
				KModManager::GetInstance()->EraseMod(entry, GetMainWindow());
			}
			else
			{
				KModManager::GetInstance()->UninstallMod(entry, GetMainWindow());
			}
		}
	}
}
void KModWorkspace::OnModSerach(wxCommandEvent& event)
{
	if (m_ViewModel->SetSearchMask(event.GetEventType() == wxEVT_SEARCHCTRL_SEARCH_BTN ? event.GetString() : wxEmptyString))
	{
		m_ViewModel->RefreshItems();
	}
}
void KModWorkspace::OnModSearchColumnsChanged(KxMenuEvent& event)
{
	KxMenuItem* item = event.GetItem();
	item->Check(!item->IsChecked());

	KxDataViewColumn::Vector columns;
	for (const auto& item: event.GetMenu()->GetMenuItems())
	{
		if (item->IsChecked())
		{
			columns.push_back(static_cast<KxDataViewColumn*>(static_cast<KxMenuItem*>(item)->GetClientData()));
		}
	}
	m_ViewModel->SetSearchColumns(columns);
}

void KModWorkspace::ClearControls()
{
	m_SearchBox->Clear();
	GetMainWindow()->ClearStatus();
}
void KModWorkspace::DisplayModInfo(KModEntry* entry)
{
	wxWindowUpdateLocker lock(this);
	ClearControls();

	if (entry)
	{
		GetMainWindow()->SetStatus(entry->GetName());
	}
}
void KModWorkspace::CreateViewContextMenu(KxMenu& contextMenu, KModEntry* modEntry)
{
	if (modEntry)
	{
		const bool isMultipleSelection = m_ViewModel->GetView()->GetSelectedItemsCount() > 1;
		const bool isFixedMod = modEntry->ToFixedEntry();
		const bool isPriorityGroup = modEntry->ToPriorityGroup();
		const bool isLinkedMod = modEntry->IsLinkedMod();
		const bool isInstalled = modEntry->IsInstalled();
		const bool isPackageExist = modEntry->IsInstallPackageFileExist() && !isFixedMod;
		const bool isVFSActive = KModManager::GetInstance()->IsVFSMounted();

		if (isInstalled)
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KMC_ID_MOD_UNINSTALL, KTr("ModManager.Menu.UninstallMod")));
			item->SetBitmap(KGetBitmap(KIMG_BOX_MINUS));
			item->Enable(!isMultipleSelection && !isVFSActive && !isFixedMod);
		}
		else
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KMC_ID_MOD_INSTALL, KTr("ModManager.Menu.InstallMod")));
			item->SetBitmap(KGetBitmap(KIMG_BOX));
			item->Enable(!isMultipleSelection && isPackageExist && !isFixedMod);
		}
		{
			// Linked mods can't be uninstalled on erase
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KMC_ID_MOD_UNINSTALL_AND_ERASE));
			item->SetItemLabel(isInstalled && !isLinkedMod ? KTr("ModManager.Menu.UninstallAndErase") : KTr("ModManager.Menu.Erase"));
			item->SetBitmap(KGetBitmap(KIMG_ERASER));
			item->Enable(!isMultipleSelection && !isFixedMod && !isVFSActive);
		}
		contextMenu.AddSeparator();

		{
			/* Image menu */
			KxMenu* imageMenu = new KxMenu();
			contextMenu.Add(imageMenu, KTr("ModManager.Menu.Image"));

			{
				KxMenuItem* item = imageMenu->Add(new KxMenuItem(KMC_ID_MOD_IMAGE_SHOW, KTr("ModManager.Menu.Image.Show")));
				item->Enable(!isMultipleSelection && !isFixedMod);
			}
			{
				KxMenuItem* item = imageMenu->Add(new KxMenuItem(KMC_ID_MOD_IMAGE_ASSIGN, KTr("ModManager.Menu.Image.Assign")));
				item->SetBitmap(KGetBitmap(KIMG_IMAGE));
				item->Enable(!isMultipleSelection && !isFixedMod);
			}
			imageMenu->AddSeparator();

			auto AddOption = [this, imageMenu](ImageResizeMode mode, const wxString& anme)
			{
				KxMenuItem* item = imageMenu->Add(new KxMenuItem(anme, wxEmptyString, wxITEM_RADIO));
				item->Check(mode == m_ImageResizeMode);
				item->Bind(KxEVT_MENU_SELECT, [this, mode](KxMenuEvent& event)
				{
					ChangeImageResizeMode(mode);
				});
			};
			AddOption(ImageResizeMode::Scale, KTr("ModManager.Menu.Image.ScaleAspect"));
			AddOption(ImageResizeMode::Stretch, KTr("ModManager.Menu.Image.Stretch"));
			AddOption(ImageResizeMode::Fill, KTr("ModManager.Menu.Image.Fill"));
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KMC_ID_MOD_EDIT_DESCRIPTION, KTr("ModManager.Menu.EditDescription")));
			item->SetBitmap(KGetBitmap(KIMG_DOCUMENT_PENICL));
			item->Enable(!isMultipleSelection && !isFixedMod);
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KMC_ID_MOD_EDIT_TAGS, KTr("ModManager.Menu.EditTags")));
			item->SetBitmap(KGetBitmap(KIMG_TAGS));
			item->Enable(!isFixedMod);
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KMC_ID_MOD_EDIT_SITES, KTr("ModManager.Menu.EditSites")));
			item->SetBitmap(KGetBitmap(KNetworkProvider::GetGenericIcon()));
			item->Enable(!isMultipleSelection && !isFixedMod);
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KMC_ID_MOD_CHANGE_ID, KTr("ModManager.Menu.ChangeID")));
			item->SetBitmap(KGetBitmap(KIMG_KEY));
			item->Enable(!isMultipleSelection && !isVFSActive && !isFixedMod);
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KMC_ID_MOD_EXPLORE_FILES, KTr("ModManager.Menu.ExploreFiles")));
			item->Enable(!isMultipleSelection && !isPriorityGroup);
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KMC_ID_MOD_SHOW_COLLISIONS, KTr("ModManager.Menu.ShowCollisions")));
			item->Enable(!isMultipleSelection && !isPriorityGroup);
		}

		/* Package menu */
		{
			KxMenu* packageMenu = new KxMenu();
			KxMenuItem* packageMenuItem = contextMenu.Add(packageMenu, KTr("ModManager.Menu.Package"));
			packageMenuItem->Enable(!isMultipleSelection && !isFixedMod && !isPriorityGroup);

			{
				KxMenuItem* item = packageMenu->Add(new KxMenuItem(KMC_ID_PACKAGE_OPEN, KTr("ModManager.Menu.Package.Open")));
				item->SetDefault();
				item->Enable(isPackageExist);
			}
			{
				KxMenuItem* item = packageMenu->Add(new KxMenuItem(KMC_ID_PACKAGE_OPEN_LOCATION, KTr("ModManager.Menu.Package.OpenLocation")));
				item->SetBitmap(KGetBitmap(KIMG_FOLDER_OPEN));
				item->Enable(isPackageExist);
			}
			packageMenu->AddSeparator();

			{
				KxMenuItem* item = packageMenu->Add(new KxMenuItem(KMC_ID_PACKAGE_ASSIGN, KTr("ModManager.Menu.Package.Assign")));
				item->SetBitmap(KGetBitmap(KIMG_BOX_SEARCH_RESULT));
				item->Enable(!isFixedMod);
			}
			{
				KxMenuItem* item = packageMenu->Add(new KxMenuItem(KMC_ID_PACKAGE_REMOVE, KTr("ModManager.Menu.Package.Remove")));
				item->Enable(isPackageExist);
			}
			{
				KxMenuItem* item = packageMenu->Add(new KxMenuItem(KMC_ID_PACKAGE_EXTRACT, KTr("ModManager.Menu.Package.Extract")));
				item->Enable(isPackageExist);
			}
			{
				KxMenuItem* item = packageMenu->Add(new KxMenuItem(KMC_ID_PACKAGE_IMPORT_PROJECT, KTr("ModManager.Menu.Package.ImportProject")));
				item->Enable(isPackageExist);
			}
			{
				KxMenuItem* item = packageMenu->Add(new KxMenuItem(KMC_ID_PACKAGE_CREATE_PROJECT, KTr("ModManager.Menu.Package.CreateProject")));
				item->Enable(isPackageExist);
			}
			packageMenu->AddSeparator();

			{
				KxMenuItem* item = packageMenu->Add(new KxMenuItem(KMC_ID_PACKAGE_PROPERTIES, KTr("ModManager.Menu.Properties")));
				item->Enable(isPackageExist);
			}
		}
		contextMenu.AddSeparator();

		// Color menu
		{
			KxMenu* colorMenu = new KxMenu();
			contextMenu.Add(colorMenu, KTr("Generic.Color"));

			{
				KxMenuItem* item = colorMenu->Add(new KxMenuItem(KMC_COLOR_ASSIGN, KTr("Generic.Assign")));
				item->SetDefault();
			}
			{
				KxMenuItem* item = colorMenu->Add(new KxMenuItem(KMC_COLOR_RESET, KTr("Generic.Reset")));
				item->Enable(modEntry->HasColor() || m_ViewModel->GetView()->GetSelectedItemsCount() != 0);
			}
		}
		contextMenu.AddSeparator();

		// Other items
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KMC_ID_MOD_OPEN_LOCATION, KTr("ModManager.Menu.OpenModFilesLocation")));
			item->SetBitmap(KGetBitmap(KIMG_FOLDER_OPEN));
			item->Enable(!isMultipleSelection && !isPriorityGroup);
		}
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KMC_ID_MOD_CHANGE_LOCATION, KTr("ModManager.Menu.ChangeModFilesLocation")));
			item->SetBitmap(KGetBitmap(KIMG_FOLDER_ARROW));
			item->Enable(!isMultipleSelection && !isFixedMod);
		}
		if (!isMultipleSelection && isLinkedMod && !isFixedMod)
		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KMC_ID_MOD_REVERT_LOCATION, KTr("ModManager.Menu.RevertModFilesLocation")));
			item->SetBitmap(KGetBitmap(KIMG_FOLDER_ARROW));
		}
		contextMenu.AddSeparator();

		{
			KxMenuItem* item = contextMenu.Add(new KxMenuItem(KMC_ID_MOD_PROPERTIES, KTr("ModManager.Menu.Properties")));
			item->Enable(!isMultipleSelection && !isPriorityGroup);
		}
	}

	/* Generic items */
	if (contextMenu.GetMenuItemCount() != 0)
	{
		contextMenu.AddSeparator();
	}

	{
		KxMenuItem* item = contextMenu.Add(new KxMenuItem(KTr(KxID_REFRESH)));
		item->SetBitmap(KGetBitmap(KIMG_ARROW_CIRCLE_DOUBLE));
		item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
		{
			KModManager::GetInstance()->Load();
			KModManager::GetInstance()->ResortMods();
			ReloadWorkspace();
		});
	}
}
void KModWorkspace::ChangeImageResizeMode(ImageResizeMode mode)
{
	m_ImageResizeMode = mode;
	for (auto& mod: KModManager::GetInstance()->GetAllEntries())
	{
		mod->ResetBitmap();
	}
	m_ViewModel->UpdateUI();
}

void KModWorkspace::SelectMod(const KModEntry* entry)
{
	m_ViewModel->SelectMod(entry);
}
void KModWorkspace::ProcessSelection(KModEntry* entry)
{
	DisplayModInfo(entry);
}
void KModWorkspace::HighlightMod(const KModEntry* entry)
{
	if (entry)
	{
		KxDataViewItem item = m_ViewModel->GetItemByEntry(entry);
		m_ViewModel->GetView()->Select(item);
		m_ViewModel->GetView()->EnsureVisible(item);
	}
	else
	{
		m_ViewModel->GetView()->UnselectAll();
	}
}
void KModWorkspace::ReloadView()
{
	m_ViewModel->RefreshItems();
	ProcessSelection();
}

void KModWorkspace::ShowViewContextMenu(KModEntry* modEntry)
{
	// Get mouse position before doing anything else,
	// as mouse pointer can move before displaying showing the menu.
	wxPoint mousePos = wxGetMousePosition();

	// Show menu
	KxMenu contextMenu;
	CreateViewContextMenu(contextMenu, modEntry);

	wxWindowID menuID = contextMenu.Show(NULL, mousePos);
	switch (menuID)
	{
		// Mod
		case KMC_ID_MOD_OPEN_LOCATION:
		{
			wxString location = modEntry->IsInstalled() ? modEntry->GetModFilesDir() : modEntry->GetRootDir();
			KxShell::Execute(GetMainWindow(), location, "open");
			break;
		}
		case KMC_ID_MOD_CHANGE_LOCATION:
		{
			KxFileBrowseDialog dialog(GetMainWindow(), KxID_NONE, KxFBD_OPEN_FOLDER);
			if (dialog.ShowModal() == KxID_OK)
			{
				if (!modEntry->IsInstalled() || KxShell::FileOperationEx(KxFOF_MOVE, modEntry->GetModFilesDir() + "\\*", dialog.GetResult(), this, true, false, false, true))
				{
					KxFile(modEntry->GetModFilesDir()).RemoveFolder(true);
					modEntry->SetLinkedModLocation(dialog.GetResult());
					modEntry->Save();

					KModEvent(KEVT_MOD_FILES_CHANGED, *modEntry).Send();
					KModEvent(KEVT_MOD_CHANGED, *modEntry).Send();
				}
			}
			break;
		}
		case KMC_ID_MOD_REVERT_LOCATION:
		{
			if (!modEntry->IsInstalled() || KxShell::FileOperationEx(KxFOF_MOVE, modEntry->GetModFilesDir() + "\\*", modEntry->GetDefaultModFilesDir(), this, true, false, false, true))
			{
				// Remove file folder if it's empty
				KxFile(modEntry->GetModFilesDir()).RemoveFolder(true);

				modEntry->SetLinkedModLocation(wxEmptyString);
				modEntry->Save();

				KModEvent(KEVT_MOD_FILES_CHANGED, *modEntry).Send();
				KModEvent(KEVT_MOD_CHANGED, *modEntry).Send();
			}
			break;
		}
		case KMC_ID_MOD_INSTALL:
		{
			InstallMod(modEntry);
			break;
		}
		case KMC_ID_MOD_UNINSTALL:
		case KMC_ID_MOD_UNINSTALL_AND_ERASE:
		{
			UninstallMod(modEntry, menuID == KMC_ID_MOD_UNINSTALL_AND_ERASE);
			break;
		}
		case KMC_ID_MOD_IMAGE_SHOW:
		{
			KImageViewerDialog dialog(this);

			KImageViewerEvent event(wxEVT_NULL, modEntry->GetImageFile());
			dialog.Navigate(event);
			dialog.ShowModal();
			break;
		}
		case KMC_ID_MOD_IMAGE_ASSIGN:
		{
			KxFileBrowseDialog dialog(GetMainWindow(), KxID_NONE, KxFBD_OPEN);
			dialog.AddFilter(KxString::Join(KScreenshotsGalleryManager::GetSupportedExtensions(), ";"), KTr("FileFilter.Images"));
			if (dialog.ShowModal() == KxID_OK)
			{
				KxFile(dialog.GetResult()).CopyFile(modEntry->GetImageFile(), true);
				modEntry->ResetBitmap();
				modEntry->ResetNoBitmap();
				modEntry->Save();

				KModEvent(KEVT_MOD_CHANGED, *modEntry).Send();
			}
			break;
		}
		case KMC_ID_MOD_EDIT_DESCRIPTION:
		{
			wxString oldDescription = modEntry->GetDescription();
			KTextEditorDialog dialog(GetMainWindow());
			dialog.SetText(oldDescription);

			if (dialog.ShowModal() == KxID_OK && dialog.IsModified())
			{
				modEntry->SetDescription(dialog.GetText());
				modEntry->Save();

				KModEvent event(KEVT_MOD_CHANGED, *modEntry);
				ProcessEvent(event);
			}
			break;
		}
		case KMC_ID_MOD_EDIT_TAGS:
		{
			KModEntry tempEntry(*modEntry);
			KModTagsSelectorDialog dialog(GetMainWindow(), KTr("ModManager.TagsDialog"));
			dialog.SetDataVector(&tempEntry.GetTags(), &tempEntry);
			dialog.ShowModal();
			if (dialog.IsModified())
			{
				dialog.ApplyChanges();
				bool hasSelection = DoForAllSelectedItems(m_ViewModel, [&tempEntry](KModEntry& entry)
				{
					entry.GetTags() = tempEntry.GetTags();
					entry.SetPriorityGroupTag(tempEntry.GetPriorityGroupTag());
					entry.Save();

					KModEvent(KEVT_MOD_CHANGED, entry).Send();
					return true;
				});
				if (hasSelection)
				{
					ReloadView();
				}
			}
			break;
		}
		case KMC_ID_MOD_EDIT_SITES:
		{
			KModSitesEditorDialog dialog(GetMainWindow(), modEntry->GetWebSites(), modEntry->GetFixedWebSites());
			dialog.ShowModal();
			if (dialog.IsModified())
			{
				modEntry->Save();
				KModEvent(KEVT_MOD_CHANGED, *modEntry).Send();
			}
			break;
		}
		case KMC_ID_MOD_CHANGE_ID:
		{
			if (ShowChangeModIDDialog(modEntry))
			{
				m_ViewModel->RefreshItem(modEntry);
			}
			break;
		}
		case KMC_ID_MOD_EXPLORE_FILES:
		{
			KModFilesExplorerDialog dialog(this, *modEntry);
			dialog.ShowModal();
			break;
		}
		case KMC_ID_MOD_SHOW_COLLISIONS:
		{
			new KModCollisionViewerModelDialog(this, modEntry);
			break;
		}
		case KMC_ID_MOD_PROPERTIES:
		{
			KxShell::Execute(GetMainWindow(), modEntry->GetModFilesDir(), "properties");
			break;
		}

		// Package
		case KMC_ID_PACKAGE_OPEN:
		{
			OpenPackage(modEntry->GetInstallPackageFile());
			break;
		}
		case KMC_ID_PACKAGE_OPEN_LOCATION:
		{
			KxShell::OpenFolderAndSelectItem(modEntry->GetInstallPackageFile());
			break;
		}
		case KMC_ID_PACKAGE_ASSIGN:
		{
			KxFileBrowseDialog dialog(GetMainWindow(), KxID_NONE, KxFBD_OPEN);
			dialog.SetFolder(modEntry->GetInstallPackageFile().BeforeLast('\\'));
			dialog.AddFilter("*.kmp;*.smi;*.fomod;*.7z;*.zip;", KTr("FileFilter.AllSupportedFormats"));
			dialog.AddFilter("*.kmp", KTr("FileFilter.ModPackage"));
			dialog.AddFilter("*.smi", KTr("FileFilter.ModPackageSMI"));
			dialog.AddFilter("*.fomod", KTr("FileFilter.ModPackageFOMod"));
			dialog.AddFilter("*.7z;*.zip;", KTr("FileFilter.Archives"));
			if (dialog.ShowModal() == KxID_OK)
			{
				modEntry->SetInstallPackageFile(dialog.GetResult());
				modEntry->Save();

				KModEvent(KEVT_MOD_CHANGED, *modEntry).Send();
			}
			break;
		}
		case KMC_ID_PACKAGE_REMOVE:
		{
			KxShell::FileOperation(modEntry->GetInstallPackageFile(), KxFS_FILE, KxFOF_DELETE, true, false, this);
			break;
		}
		case KMC_ID_PACKAGE_EXTRACT:
		{
			KxFileBrowseDialog dialog(GetMainWindow(), KxID_NONE, KxFBD_OPEN_FOLDER);
			if (dialog.ShowModal() == KxID_OK)
			{
				// Extract archive in mod name folder inside the specified one.
				wxString outPath = dialog.GetResult() + '\\' + modEntry->GetSafeName();

				KPackageManager::ExtractAcrhiveThreaded(this, modEntry->GetInstallPackageFile(), outPath);
			}
			break;
		}
		case KMC_ID_PACKAGE_IMPORT_PROJECT:
		{
			KPackageCreatorWorkspace::GetInstance()->CreateNow();
			KPackageCreatorWorkspace::GetInstance()->ImportProjectFromPackage(modEntry->GetInstallPackageFile());
			KPackageCreatorWorkspace::GetInstance()->SwitchHere();
			break;
		}
		case KMC_ID_PACKAGE_CREATE_PROJECT:
		{
			KPackageCreatorWorkspace::GetInstance()->CreateNow();
			KPackageCreatorWorkspace::GetInstance()->CreateProjectFromModEntry(*modEntry);
			KPackageCreatorWorkspace::GetInstance()->SwitchHere();
			break;
		}

		// Color menu
		case KMC_COLOR_ASSIGN:
		{
			wxColourData colorData;
			colorData.SetColour(modEntry->GetColor());
			colorData.SetChooseFull(true);
			colorData.SetChooseAlpha(true);

			wxColourDialog dialog(this, &colorData);
			if (dialog.ShowModal() == wxID_OK)
			{
				DoForAllSelectedItems(m_ViewModel, [&dialog](KModEntry& entry)
				{
					entry.SetColor(dialog.GetColourData().GetColour());
					entry.Save();

					KModEvent(KEVT_MOD_CHANGED, entry).Send();
					return true;
				});
				m_ViewModel->UpdateUI();
			}
			break;
		}
		case KMC_COLOR_RESET:
		{
			DoForAllSelectedItems(m_ViewModel, [](KModEntry& entry)
			{
				entry.SetColor(KxColor());
				entry.Save();

				KModEvent(KEVT_MOD_CHANGED, entry).Send();
				return true;
			});
			m_ViewModel->UpdateUI();
			break;
		}

		case KMC_ID_PACKAGE_PROPERTIES:
		{
			KxShell::Execute(GetMainWindow(), modEntry->GetInstallPackageFile(), "properties");
			break;
		}
	};
}
void KModWorkspace::ShowViewContextMenu(const KModTag* modTag)
{
	if (modTag)
	{
		KxMenu menu;
		menu.Add(new KxMenuItem(KMC_ID_TAG_ENABLE_ALL, KTr("ModManager.Menu.Tag.ActivateAll")))->SetBitmap(KGetBitmap(KIMG_TICK_CIRCLE_FRAME));
		menu.Add(new KxMenuItem(KMC_ID_TAG_DISABLE_ALL, KTr("ModManager.Menu.Tag.DeactivateAll")))->SetBitmap(KGetBitmap(KIMG_TICK_CIRCLE_FRAME_EMPTY));

		// State
		bool isVFSActive = KModManager::GetInstance()->IsVFSMounted();
		for (auto& item: menu.GetMenuItems())
		{
			item->Enable(!isVFSActive);
		}

		wxWindowID menuID = menu.Show();
		switch (menuID)
		{
			case KMC_ID_TAG_ENABLE_ALL:
			case KMC_ID_TAG_DISABLE_ALL:
			{
				auto CheckTag = [modTag](const KModEntry& modEntry)
				{
					if (modTag->HasValue())
					{
						return KModEntry::HasTag(modEntry.GetTags(), modTag->GetValue());
					}
					else
					{
						return modEntry.GetTags().empty();
					}
				};

				for (auto& modEntry: KModManager::GetInstance()->GetEntries())
				{
					if (modEntry->IsInstalled() && CheckTag(*modEntry))
					{
						modEntry->SetEnabled(menuID == KMC_ID_TAG_ENABLE_ALL);
						modEntry->Save();
					}
				}
				break;
			}
		};
	}
}
void KModWorkspace::UpdateModListContent()
{
	m_ToolBar_Profiles->Clear();
	int selectIndex = 0;
	for (const auto& profile: KGameInstance::GetActive()->GetProfiles())
	{
		int index = m_ToolBar_Profiles->Append(profile->GetID());
		if (profile->GetID() == KGameInstance::GetActiveProfileID())
		{
			selectIndex = index;
		}
	}
	m_ToolBar_Profiles->SetSelection(selectIndex);
}

bool KModWorkspace::IsAnyChangeAllowed() const
{
	return IsMovingModsAllowed();
}
bool KModWorkspace::IsMovingModsAllowed() const
{
	return m_ViewModel->GetDisplayMode() == KMM_TYPE_CONNECTOR && IsChangingModsAllowed();
}
bool KModWorkspace::IsChangingModsAllowed() const
{
	return !KModManager::GetInstance()->IsVFSMounted();
}
