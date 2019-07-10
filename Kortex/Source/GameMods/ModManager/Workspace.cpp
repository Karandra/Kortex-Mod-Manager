#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/ApplicationOptions.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/ModImporter.hpp>
#include <Kortex/ModStatistics.hpp>
#include <Kortex/ModTagManager.hpp>
#include <Kortex/SaveManager.hpp>
#include <Kortex/ScreenshotsGallery.hpp>
#include <Kortex/ProgramManager.hpp>
#include <Kortex/InstallWizard.hpp>
#include <Kortex/VirtualGameFolder.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/Events.hpp>

#include "GameInstance/ProfileEditor.h"
#include "DisplayModel.h"
#include "DefaultModManager.h"
#include "BasicGameMod.h"
#include "NewModDialog.h"
#include "GameMods/KModFilesExplorerDialog.h"
#include "GameMods/KModCollisionViewerModel.h"
#include "UI/KImageViewerDialog.h"
#include "UI/TextEditDialog.h"
#include "Utility/KOperationWithProgress.h"
#include "Utility/KAux.h"
#include "Utility/MenuSeparator.h"
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

namespace Kortex::Application::OName
{
	KortexDefOption(Splitter);

	KortexDefOption(ShowPriorityGroups);
	KortexDefOption(ShowNotInstalledMods);
	KortexDefOption(BoldPriorityGroupLabels);
	KortexDefOption(PriorityGroupLabelAlignment);
	KortexDefOption(DisplayMode);
	KortexDefOption(ImageResizeMode);
}

namespace
{
	using namespace Kortex;
	using namespace Kortex::ModManager;
	using namespace Kortex::Application;

	enum class DisplayModeMenuID
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
	enum class ToolsMenuID
	{
		Statistics = KxID_HIGHEST + 1,
		ExportModList,
	};
	enum class ContextMenuID
	{
		BeginIndex = KxID_HIGHEST,

		ModOpenLocation,
		ModChangeLocation,
		ModRevertLocation,

		ModInstall,
		ModUninstall,
		ModErase,

		ModImageShow,
		ModImageAssign,

		ModEditDescription,
		ModEditTags,
		ModEditSources,
		ModChangeID,
		ModExploreFiles,
		ModShowCollisions,
		ModProperties,

		ColorAssign,
		ColorReset,
	};

	template<class Functor> bool DoForAllSelectedItems(const IGameMod::RefVector& selectedMods, Functor&& func)
	{
		if (!selectedMods.empty())
		{
			bool result = false;
			for (IGameMod* gameMod: selectedMods)
			{
				if (!func(*gameMod))
				{
					break;
				}
				result = true;
			}
			return result;
		}
		return false;
	}

	auto GetDisplayModelOptions()
	{
		return Application::GetAInstanceOptionOf<IModManager>(OName::Workspace, OName::DisplayModel);
	}
	auto GetSplitterOptions()
	{
		return Application::GetAInstanceOptionOf<IModManager>(OName::Workspace, OName::Splitter);
	}
}

namespace Kortex::ModManager
{
	Workspace::Workspace(KMainWindow* mainWindow)
		:KWorkspace(mainWindow)//, m_OptionsUI(this, "MainUI"), m_ModListViewOptions(this, "ModListView")
	{
		CreateToolBarButton();
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
	}
	Workspace::~Workspace()
	{
		if (IsWorkspaceCreated())
		{
			auto options = GetDisplayModelOptions();
			options.SaveDataViewLayout(m_DisplayModel->GetView());

			options.SetAttribute(OName::DisplayMode, (int)m_DisplayModel->GetDisplayMode());
			options.SetAttribute(OName::ShowPriorityGroups, m_DisplayModel->ShouldShowPriorityGroups());
			options.SetAttribute(OName::ShowNotInstalledMods, m_DisplayModel->ShouldShowNotInstalledMods());
			options.SetAttribute(OName::BoldPriorityGroupLabels, m_DisplayModel->IsBoldPriorityGroupLabels());
			options.SetAttribute(OName::PriorityGroupLabelAlignment, (int)m_DisplayModel->GetPriorityGroupLabelAlignment());

			GetSplitterOptions().SaveSplitterLayout(m_SplitterLeftRight);
		}
	}
	bool Workspace::OnCreateWorkspace()
	{
		m_SplitterLeftRight = new KxSplitterWindow(this, KxID_NONE);
		m_SplitterLeftRight->SetName("Horizontal");
		m_SplitterLeftRight->SetMinimumPaneSize(250);
		m_MainSizer->Add(m_SplitterLeftRight, 1, wxEXPAND);
		IThemeManager::GetActive().ProcessWindow(m_SplitterLeftRight, true);

		// Mods Panel
		m_ModsPaneSizer = new wxBoxSizer(wxVERTICAL);
		m_ModsPane = new KxPanel(m_SplitterLeftRight, KxID_NONE);
		m_ModsPane->SetSizer(m_ModsPaneSizer);
		m_ModsPane->SetBackgroundColour(GetBackgroundColour());

		CreateModsView();
		CreateToolBar();
		m_ModsPaneSizer->Add(m_ModsToolBar, 0, wxEXPAND);
		m_ModsPaneSizer->Add(m_DisplayModel->GetView(), 1, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);

		CreateRightPane();
		CreateControls();

		ReloadView();
		UpdateModListContent();

		IEvent::Bind(Events::MainVFSToggled, &Workspace::OnVFSToggled, this);
		IEvent::Bind(Events::ProfileSelected, &Workspace::OnProfileSelected, this);

		CreateAsSubWorkspace<VirtualGameFolder::Workspace>();
		CreateAsSubWorkspace<ProgramManager::Workspace>();
		return true;
	}

	void Workspace::CreateToolBar()
	{
		m_ModsToolBar = new KxAuiToolBar(m_ModsPane, KxID_NONE, wxAUI_TB_HORZ_TEXT|wxAUI_TB_PLAIN_BACKGROUND);
		m_ModsToolBar->SetBackgroundColour(GetBackgroundColour());
		m_ModsToolBar->SetMargins(0, 1, 0, 0);

		m_ToolBar_Profiles = new KxComboBox(m_ModsToolBar, KxID_NONE);
		m_ToolBar_Profiles->Bind(wxEVT_COMBOBOX, &Workspace::OnSelectProfile, this);

		m_ModsToolBar->AddLabel(KTr("ModManager.Profile") + ':');
		m_ModsToolBar->AddControl(m_ToolBar_Profiles)->SetProportion(1);

		m_ToolBar_EditProfiles = KMainWindow::CreateToolBarButton(m_ModsToolBar, wxEmptyString, ImageResourceID::Gear);
		m_ToolBar_EditProfiles->SetShortHelp(KTr("ModManager.Profile.Configure"));
		m_ToolBar_EditProfiles->Bind(KxEVT_AUI_TOOLBAR_CLICK, &Workspace::OnShowProfileEditor, this);
		m_ModsToolBar->AddSeparator();

		m_ToolBar_AddMod = KMainWindow::CreateToolBarButton(m_ModsToolBar, KTr(KxID_ADD), ImageResourceID::PlusSmall);
	
		m_ToolBar_ChangeDisplayMode = KMainWindow::CreateToolBarButton(m_ModsToolBar, KTr("ModManager.DisplayMode.Caption"), ImageResourceID::ProjectionScreen);
		m_ToolBar_ChangeDisplayMode->Bind(KxEVT_AUI_TOOLBAR_CLICK, &Workspace::OnDisplayModeMenu, this);
	
		m_ToolBar_Tools = KMainWindow::CreateToolBarButton(m_ModsToolBar, wxEmptyString, ImageResourceID::WrenchScrewdriver);
		m_ToolBar_Tools->SetShortHelp(KTr("ModManager.Tools"));
		m_ToolBar_Tools->Bind(KxEVT_AUI_TOOLBAR_CLICK, &Workspace::OnToolsMenu, this);

		m_SearchBox = new KxSearchBox(m_ModsToolBar, KxID_NONE);
		m_SearchBox->SetName("Workspace search box");
		m_SearchBox->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &Workspace::OnModSerach, this);
		m_SearchBox->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &Workspace::OnModSerach, this);

		KxMenu* searchMenu = new KxMenu();
		searchMenu->Bind(KxEVT_MENU_SELECT, &Workspace::OnModSearchColumnsChanged, this);
		m_DisplayModel->CreateSearchColumnsMenu(*searchMenu);
		m_SearchBox->SetMenu(searchMenu);

		m_ModsToolBar->AddControl(m_SearchBox);

		m_ModsToolBar->Realize();

		CreateDisplayModeMenu();
		CreateAddModMenu();
		CreateToolsMenu();
	}
	void Workspace::CreateDisplayModeMenu()
	{
		m_ToolBar_DisplayModeMenu = new KxMenu();
		m_ToolBar_ChangeDisplayMode->AssignDropdownMenu(m_ToolBar_DisplayModeMenu);
		m_ToolBar_ChangeDisplayMode->SetOptionEnabled(KxAUI_TBITEM_OPTION_LCLICK_MENU);

		{
			KxMenuItem* item = m_ToolBar_DisplayModeMenu->Add(new KxMenuItem((int)DisplayModeMenuID::Connector, KTr("ModManager.DisplayMode.Connector"), wxEmptyString, wxITEM_RADIO));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::PlugDisconnect));
			item->Check(m_DisplayModel->GetDisplayMode() == DisplayModelType::Connector);
		}
		{
			KxMenuItem* item = m_ToolBar_DisplayModeMenu->Add(new KxMenuItem((int)DisplayModeMenuID::Manager, KTr("ModManager.DisplayMode.Log"), wxEmptyString, wxITEM_RADIO));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Category));
			item->Check(m_DisplayModel->GetDisplayMode() == DisplayModelType::Manager);
		}
		m_ToolBar_DisplayModeMenu->AddSeparator();

		{
			KxMenuItem* item = m_ToolBar_DisplayModeMenu->Add(new KxMenuItem((int)DisplayModeMenuID::ShowPriorityGroups, KTr("ModManager.DisplayMode.ShowPriorityGroups"), wxEmptyString, wxITEM_CHECK));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Folders));
			item->Check(m_DisplayModel->ShouldShowPriorityGroups());
		}
		{
			KxMenuItem* item = m_ToolBar_DisplayModeMenu->Add(new KxMenuItem((int)DisplayModeMenuID::ShowNotInstalledMods, KTr("ModManager.DisplayMode.ShowNotInstalledMods"), wxEmptyString, wxITEM_CHECK));
			item->Check(m_DisplayModel->ShouldShowNotInstalledMods());
		}
		{
			KxMenuItem* item = m_ToolBar_DisplayModeMenu->Add(new KxMenuItem((int)DisplayModeMenuID::BoldPriorityGroupLabels, KTr("ModManager.DisplayMode.BoldPriorityGroupLabels"), wxEmptyString, wxITEM_CHECK));
			item->Check(m_DisplayModel->IsBoldPriorityGroupLabels());
		}
		m_ToolBar_DisplayModeMenu->AddSeparator();

		{
			KxMenu* alignmnetMenu = new KxMenu();
			m_ToolBar_DisplayModeMenu->Add(alignmnetMenu, KTr("ModManager.DisplayMode.PriorityGroupLabelsAlignment"));

			using PriorityGroupLabelAlignment = DisplayModel::PriorityGroupLabelAlignment;
			auto AddOption = [this, alignmnetMenu](DisplayModeMenuID id, PriorityGroupLabelAlignment type, KxStandardID trId)
			{
				KxMenuItem* item = alignmnetMenu->Add(new KxMenuItem((int)id, KTr(trId), wxEmptyString, wxITEM_RADIO));
				item->Check(m_DisplayModel->GetPriorityGroupLabelAlignment() == type);
				return item;
			};
			AddOption(DisplayModeMenuID::PriorityGroupLabelAlignment_Left, PriorityGroupLabelAlignment::Left, KxID_JUSTIFY_LEFT);
			AddOption(DisplayModeMenuID::PriorityGroupLabelAlignment_Center, PriorityGroupLabelAlignment::Center, KxID_JUSTIFY_CENTER);
			AddOption(DisplayModeMenuID::PriorityGroupLabelAlignment_Right, PriorityGroupLabelAlignment::Right, KxID_JUSTIFY_RIGHT);
		}
	}
	void Workspace::CreateAddModMenu()
	{
		m_ToolBar_AddModMenu = new KxMenu();
		m_ToolBar_AddMod->AssignDropdownMenu(m_ToolBar_AddModMenu);
		m_ToolBar_AddMod->SetOptionEnabled(KxAUI_TBITEM_OPTION_LCLICK_MENU);
		KxMenuItem* item = nullptr;

		item = m_ToolBar_AddModMenu->Add(new KxMenuItem(KTr("ModManager.NewMod.NewEmptyMod")));
		item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FolderPlus));
		item->Bind(KxEVT_MENU_SELECT, &Workspace::OnAddMod_Empty, this);

		item = m_ToolBar_AddModMenu->Add(new KxMenuItem(KTr("ModManager.NewMod.NewModFromFolder")));
		item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FolderArrow));
		item->Bind(KxEVT_MENU_SELECT, &Workspace::OnAddMod_FromFolder, this);
	
		item = m_ToolBar_AddModMenu->Add(new KxMenuItem(KTr("ModManager.NewMod.InstallPackage")));
		item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::BoxSearchResult));
		item->Bind(KxEVT_MENU_SELECT, &Workspace::OnAddMod_InstallPackage, this);

		m_ToolBar_AddModMenu->AddSeparator();

		item = m_ToolBar_AddModMenu->Add(new KxMenuItem(KTrf("ModManager.NewMod.ImportFrom", "Mod Organizer")));
		item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::MO2));
		item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
		{
			IModImporter::PerformImport(IModImporter::Type::ModOrganizer, this);
		});

		item = m_ToolBar_AddModMenu->Add(new KxMenuItem(KTrf("ModManager.NewMod.ImportFrom", "Nexus Mod Manager")));
		item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::ModNetwork_Nexus));
		item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
		{
			IModImporter::PerformImport(IModImporter::Type::NexusModManager, this);
		});
	}
	void Workspace::CreateToolsMenu()
	{
		KxMenu* menu = new KxMenu();
		m_ToolBar_Tools->AssignDropdownMenu(menu);
		m_ToolBar_Tools->SetOptionEnabled(KxAUI_TBITEM_OPTION_LCLICK_MENU);
		KxMenuItem* item = nullptr;

		item = menu->Add(new KxMenuItem((int)ToolsMenuID::Statistics, KTr("ModManager.Statistics")));
		item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Chart));

		item = menu->Add(new KxMenuItem((int)ToolsMenuID::ExportModList, KTr("Generic.Export")));
		item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Disk));
	}

	void Workspace::CreateModsView()
	{
		const auto options = GetDisplayModelOptions();

		m_DisplayModel = new DisplayModel();
		m_DisplayModel->ShowPriorityGroups(options.GetAttributeBool(OName::ShowPriorityGroups));
		m_DisplayModel->ShowNotInstalledMods(options.GetAttributeBool(OName::ShowNotInstalledMods));
		m_DisplayModel->SetBoldPriorityGroupLabels(options.GetAttributeBool(OName::BoldPriorityGroupLabels));
		m_DisplayModel->SetPriorityGroupLabelAlignment((DisplayModel::PriorityGroupLabelAlignment)options.GetAttributeInt(OName::PriorityGroupLabelAlignment));
	
		m_DisplayModel->CreateView(m_ModsPane);
		m_DisplayModel->SetDisplayMode((DisplayModelType)options.GetAttributeInt(OName::DisplayMode));
	}
	void Workspace::CreateControls()
	{
		wxBoxSizer* controlsSizer = new wxBoxSizer(wxHORIZONTAL);
		m_MainSizer->Add(controlsSizer, 0, wxEXPAND|wxALIGN_RIGHT|wxTOP, KLC_VERTICAL_SPACING);

		m_ActivateButton = new KxButton(this, KxID_NONE, KTr("ModManager.VFS.Activate"));
		m_ActivateButton->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::TickCircleFrameEmpty));
		m_ActivateButton->Bind(wxEVT_BUTTON, &Workspace::OnMountButton, this);

		controlsSizer->Add(m_ActivateButton);
	}
	void Workspace::CreateRightPane()
	{
		m_PaneRight_Tabs = new KxAuiNotebook(m_SplitterLeftRight, KxID_NONE);
		m_PaneRight_Tabs->SetImageList(&ImageProvider::GetImageList());

		m_PaneRight_Tabs->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGING, &Workspace::OnSubWorkspaceOpening, this);
		m_PaneRight_Tabs->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &Workspace::OnSubWorkspaceOpened, this);
	}

	bool Workspace::OnOpenWorkspace()
	{
		if (IsFirstTimeOpen())
		{
			m_SplitterLeftRight->SplitVertically(m_ModsPane, m_PaneRight_Tabs);
			if (m_PaneRight_Tabs->GetPageCount() == 0)
			{
				m_SplitterLeftRight->Unsplit(m_PaneRight_Tabs);
			}

			GetSplitterOptions().LoadSplitterLayout(m_SplitterLeftRight);

			auto displayModelOptions = GetDisplayModelOptions();
			displayModelOptions.LoadDataViewLayout(m_DisplayModel->GetView());

			m_DisplayModel->LoadView();
		}
		m_DisplayModel->UpdateUI();
		return true;
	}
	bool Workspace::OnCloseWorkspace()
	{
		GetMainWindow()->ClearStatus();
		return true;
	}
	void Workspace::OnReloadWorkspace()
	{
		ClearControls();
		
		m_DisplayModel->LoadView();
		ProcessSelection();
		UpdateModListContent();
	}

	wxString Workspace::GetID() const
	{
		return wxS("ModManager::Workspace");
	}
	wxString Workspace::GetName() const
	{
		return KTr("ModManager.Name");
	}

	bool Workspace::AddSubWorkspace(KWorkspace* workspace)
	{
		m_PaneRight_Tabs->InsertPage(workspace->GetTabIndex(), workspace, workspace->GetNameShort(), workspace->GetTabIndex() == 0, workspace->GetImageID().AsInt());
		return true;
	}
	void Workspace::OnSubWorkspaceOpening(wxAuiNotebookEvent& event)
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
	void Workspace::OnSubWorkspaceOpened(wxAuiNotebookEvent& event)
	{
		event.Skip();

		KWorkspace* workspace = static_cast<KWorkspace*>(m_PaneRight_Tabs->GetPage(event.GetSelection()));
		if (workspace && workspace->IsWorkspaceCreated())
		{
			event.Allow();
		}
	}

	void Workspace::OnMountButton(wxCommandEvent& event)
	{
		IVirtualFileSystem& vfs = IModManager::GetInstance()->GetFileSystem();
		if (vfs.IsEnabled())
		{
			vfs.Disable();
		}
		else
		{
			vfs.Enable();
		}
	}
	bool Workspace::ShowChangeModIDDialog(IGameMod* entry)
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
				else if (const IGameMod* existingMod = IModManager::GetInstance()->FindModByID(newID))
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
			return IModManager::GetInstance()->ChangeModID(*entry, newID);
		}
		return false;
	}

	void Workspace::ProcessSelectProfile(const wxString& newProfileID)
	{
		if (!newProfileID.IsEmpty())
		{
			IGameInstance* instance = IGameInstance::GetActive();
			IGameProfile* profile = IGameInstance::GetActiveProfile();

			if (!IGameInstance::IsActiveProfileID(newProfileID))
			{
				profile->SyncWithCurrentState();
				profile->SaveConfig();

				IGameProfile* newProfile = instance->GetProfile(newProfileID);
				if (newProfile)
				{
					instance->ChangeProfileTo(*newProfile);
				}
			}
		}
	}
	void Workspace::OnSelectProfile(wxCommandEvent& event)
	{
		ProcessSelectProfile(m_ToolBar_Profiles->GetString(event.GetSelection()));
		m_DisplayModel->GetView()->SetFocus();
	}
	void Workspace::OnShowProfileEditor(KxAuiToolBarEvent& event)
	{
		// Save current
		IGameProfile* profile = IGameInstance::GetActiveProfile();
		profile->SyncWithCurrentState();
		profile->SaveConfig();

		Kortex::ProfileEditor::Dialog dialog(this);
		dialog.ShowModal();
		if (dialog.IsModified())
		{
			ProcessSelectProfile(dialog.GetNewProfile());
			UpdateModListContent();
		}
	}

	void Workspace::OpenPackage(const wxString& path)
	{
		InstallWizard::WizardDialog* dialog = new InstallWizard::WizardDialog(GetMainWindow(), path);
		dialog->Bind(InstallWizard::KEVT_IW_DONE, [this](wxNotifyEvent& event)
		{
			ScheduleReload();
		});
	}

	void Workspace::OnDisplayModeMenu(KxAuiToolBarEvent& event)
	{
		wxWindowID id = m_ToolBar_ChangeDisplayMode->ShowDropdownMenu();
		if (id != KxID_NONE)
		{
			switch ((DisplayModeMenuID)id)
			{
				case DisplayModeMenuID::Connector:
				{
					m_DisplayModel->SetDisplayMode(DisplayModelType::Connector);
					m_DisplayModel->LoadView();
					ProcessSelection();
					break;
				}
				case DisplayModeMenuID::Manager:
				{
					m_DisplayModel->SetDisplayMode(DisplayModelType::Manager);
					m_DisplayModel->LoadView();
					ProcessSelection();
					break;
				}
				case DisplayModeMenuID::ShowPriorityGroups:
				{
					m_DisplayModel->ShowPriorityGroups(!m_DisplayModel->ShouldShowPriorityGroups());
					m_DisplayModel->LoadView();
					ProcessSelection();
					break;
				}
				case DisplayModeMenuID::ShowNotInstalledMods:
				{
					m_DisplayModel->ShowNotInstalledMods(!m_DisplayModel->ShouldShowNotInstalledMods());
					m_DisplayModel->LoadView();
					ProcessSelection();
					break;
				}
				case DisplayModeMenuID::BoldPriorityGroupLabels:
				{
					m_DisplayModel->SetBoldPriorityGroupLabels(!m_DisplayModel->IsBoldPriorityGroupLabels());
					m_DisplayModel->UpdateUI();
					break;
				}

				using PriorityGroupLabelAlignment = DisplayModel::PriorityGroupLabelAlignment;
				case DisplayModeMenuID::PriorityGroupLabelAlignment_Left:
				{
					m_DisplayModel->SetPriorityGroupLabelAlignment(PriorityGroupLabelAlignment::Left);
					m_DisplayModel->UpdateUI();
					break;
				}
				case DisplayModeMenuID::PriorityGroupLabelAlignment_Right:
				{
					m_DisplayModel->SetPriorityGroupLabelAlignment(PriorityGroupLabelAlignment::Right);
					m_DisplayModel->UpdateUI();
					break;
				}
				case DisplayModeMenuID::PriorityGroupLabelAlignment_Center:
				{
					m_DisplayModel->SetPriorityGroupLabelAlignment(PriorityGroupLabelAlignment::Center);
					m_DisplayModel->UpdateUI();
					break;
				}
			};
		}
	}
	void Workspace::OnToolsMenu(KxAuiToolBarEvent& event)
	{
		switch ((ToolsMenuID)m_ToolBar_Tools->ShowDropdownMenu())
		{
			case ToolsMenuID::Statistics:
			{
				ModStatistics::Dialog dialog(this);
				dialog.ShowModal();
				break;
			}
			case ToolsMenuID::ExportModList:
			{
				KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_SAVE);
				dialog.AddFilter("*.html", KTr("FileFilter.HTML"));
				dialog.SetDefaultExtension("html");
				dialog.SetFileName(IGameInstance::GetActive()->GetGameID() + wxS(" - ") + IGameInstance::GetActive()->GetInstanceID());

				if (dialog.ShowModal() == KxID_OK)
				{
					IModManager::GetInstance()->ExportModList(dialog.GetResult());
				}
				break;
			}
		};
	}
	void Workspace::OnVFSToggled(VFSEvent& event)
	{
		m_ToolBar_Profiles->Enable(!event.IsActivated());
		m_ToolBar_EditProfiles->SetEnabled(!event.IsActivated());

		wxWindowUpdateLocker lock(m_ActivateButton);
		if (event.IsActivated())
		{
			m_ActivateButton->SetLabel(KTr("ModManager.VFS.Deactivate"));
			m_ActivateButton->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::InformationFrameEmpty));
		}
		else
		{
			m_ActivateButton->SetLabel(KTr("ModManager.VFS.Activate"));
			m_ActivateButton->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::TickCircleFrameEmpty));
		}
		m_DisplayModel->UpdateUI();
	}
	void Workspace::OnProfileSelected(GameInstance::ProfileEvent& event)
	{
		ReloadView();
	}

	void Workspace::OnAddMod_Empty(KxMenuEvent& event)
	{
		NewModDialog dialog(this);
		if (dialog.ShowModal() == KxID_OK)
		{
			BasicGameMod entry;
			entry.SetID(dialog.GetFolderName());
			entry.CreateAllFolders();
			entry.SetInstallTime(wxDateTime::Now());
			entry.Save();

			ModEvent(Events::ModInstalled, entry).Send();
		}
	}
	void Workspace::OnAddMod_FromFolder(KxMenuEvent& event)
	{
		NewModFromFolderDialog dialog(this);
		if (dialog.ShowModal() == KxID_OK)
		{
			BasicGameMod modEntry;
			modEntry.SetID(dialog.GetFolderName());
			modEntry.CreateAllFolders();
			modEntry.SetInstallTime(wxDateTime::Now());

			if (dialog.ShouldCreateAsLinkedMod())
			{
				modEntry.LinkLocation(dialog.GetFolderPath());
			}
			modEntry.Save();

			if (!dialog.ShouldCreateAsLinkedMod())
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
					ModEvent(Events::ModInstalled, name).Send();
					ReloadWorkspace();
				});

				// Configure and run
				operation->SetDialogCaption(KTr("ModManager.NewMod.CopyDialogCaption"));
				operation->Run();
			}
			else
			{
				ModEvent(Events::ModInstalled, modEntry).Send();
			}
		}
	}
	void Workspace::OnAddMod_InstallPackage(KxMenuEvent& event)
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

	void Workspace::InstallMod(IGameMod& mod)
	{
		new InstallWizard::WizardDialog(this, mod.GetPackageFile());
	}
	void Workspace::UninstallMod(IGameMod& mod, bool eraseLog)
	{
		KxTaskDialog dialog(GetMainWindow(), KxID_NONE, wxEmptyString, KTr("ModManager.RemoveMod.Message"), KxBTN_YES|KxBTN_NO, KxICON_WARNING);
		if (mod.IsInstalled())
		{
			dialog.SetCaption(eraseLog ? KTrf("ModManager.RemoveMod.CaptionUninstallAndErase", mod.GetName()) : KTrf("ModManager.RemoveMod.CaptionUninstall", mod.GetName()));
		}
		else
		{
			dialog.SetCaption(KTrf("ModManager.RemoveMod.CaptionErase", mod.GetName()));
			eraseLog = true;
		}

		if (dialog.ShowModal() == KxID_YES)
		{
			if (eraseLog)
			{
				IModManager::GetInstance()->EraseMod(mod, GetMainWindow());
			}
			else
			{
				IModManager::GetInstance()->UninstallMod(mod, GetMainWindow());
			}
		}
	}
	void Workspace::OnModSerach(wxCommandEvent& event)
	{
		if (m_DisplayModel->SetSearchMask(event.GetEventType() == wxEVT_SEARCHCTRL_SEARCH_BTN ? event.GetString() : wxEmptyString))
		{
			m_DisplayModel->LoadView();
		}
	}
	void Workspace::OnModSearchColumnsChanged(KxMenuEvent& event)
	{
		KxMenuItem* item = event.GetItem();
		item->Check(!item->IsChecked());

		KxDataView2::Column::Vector columns;
		for (const auto& item: event.GetMenu()->GetMenuItems())
		{
			if (item->IsChecked())
			{
				columns.push_back(static_cast<KxDataView2::Column*>(static_cast<KxMenuItem*>(item)->GetClientData()));
			}
		}
		m_DisplayModel->SetSearchColumns(columns);
	}

	void Workspace::ClearControls()
	{
		m_SearchBox->Clear();
		GetMainWindow()->ClearStatus();
	}
	void Workspace::DisplayModInfo(IGameMod* entry)
	{
		wxWindowUpdateLocker lock(this);
		ClearControls();

		if (entry)
		{
			GetMainWindow()->SetStatus(entry->GetName());
		}
	}
	void Workspace::CreateViewContextMenu(KxMenu& contextMenu, const IGameMod::RefVector& selectedMods, IGameMod* focusedMod)
	{
		if (focusedMod)
		{
			Utility::MenuSeparatorAfter separator1(contextMenu);

			const bool isMultipleSelection = selectedMods.size() > 1;
			
			const bool isFixedMod = focusedMod->QueryInterface<FixedGameMod>();
			const bool isPriorityGroup = focusedMod->QueryInterface<PriorityGroup>();
			const bool isNormalMod = !isFixedMod && !isPriorityGroup;

			const bool isLinkedMod = focusedMod->IsLinkedMod();
			const bool isInstalled = focusedMod->IsInstalled();
			const bool isPackageExist = !isFixedMod && !isPriorityGroup && focusedMod->IsPackageFileExist();
			const bool isVFSActive = IModManager::GetInstance()->GetFileSystem().IsEnabled();

			// Install and uninstall
			if (isInstalled)
			{
				KxMenuItem* item = contextMenu.AddItem(ContextMenuID::ModUninstall, KTr("ModManager.Menu.UninstallMod"));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::BoxMinus));
				item->Enable(!isMultipleSelection && !isVFSActive && isNormalMod);
			}
			else
			{
				KxMenuItem* item = contextMenu.AddItem(ContextMenuID::ModInstall, KTr("ModManager.Menu.InstallMod"));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Box));
				item->Enable(!isMultipleSelection && isPackageExist && isNormalMod);
			}
			{
				// Linked mods can't be uninstalled on erase
				KxMenuItem* item = contextMenu.AddItem(ContextMenuID::ModErase);
				item->SetItemLabel(isInstalled && !isLinkedMod ? KTr("ModManager.Menu.UninstallAndErase") : KTr("ModManager.Menu.Erase"));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Eraser));
				item->Enable(!isMultipleSelection && !isVFSActive && isNormalMod);
			}
			contextMenu.AddSeparator();
			{
				KxMenuItem* item = contextMenu.AddItem(ContextMenuID::ModEditDescription, KTr("ModManager.Menu.EditDescription"));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::DocumentPencil));
				item->Enable(!isMultipleSelection && isNormalMod);
			}
			{
				KxMenuItem* item = contextMenu.AddItem(ContextMenuID::ModEditTags, KTr("ModManager.Menu.EditTags"));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Tags));
				item->Enable(!isFixedMod);
			}
			{
				KxMenuItem* item = contextMenu.AddItem(ContextMenuID::ModEditSources, KTr("ModManager.Menu.EditSites"));
				item->SetBitmap(ImageProvider::GetBitmap(IModNetwork::GetGenericIcon()));
				item->Enable(!isMultipleSelection && isNormalMod);
			}
			{
				KxMenuItem* item = contextMenu.AddItem(ContextMenuID::ModChangeID, KTr("ModManager.Menu.ChangeID"));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Key));
				item->Enable(!isMultipleSelection && !isVFSActive && isNormalMod);
			}
			{
				KxMenuItem* item = contextMenu.AddItem(ContextMenuID::ModExploreFiles, KTr("ModManager.Menu.ExploreFiles"));
				item->Enable(!isMultipleSelection && isNormalMod);
			}
			{
				KxMenuItem* item = contextMenu.AddItem(ContextMenuID::ModShowCollisions, KTr("ModManager.Menu.ShowCollisions"));
				item->Enable(!isMultipleSelection && isNormalMod);
			}

			// Package menu
			if (IPackageManager* packageManager = IPackageManager::GetInstance())
			{
				Utility::MenuSeparatorAfter separator(contextMenu);
				packageManager->OnModListMenu(contextMenu, selectedMods, focusedMod);
			}

			// Color menu
			if (KxMenu* colorMenu = new KxMenu(); true)
			{
				contextMenu.Add(colorMenu, KTr("Generic.Color"));
				{
					KxMenuItem* item = colorMenu->AddItem(ContextMenuID::ColorAssign, KTr("Generic.Assign"));
					item->SetDefault();
				}
				{
					KxMenuItem* item = colorMenu->AddItem(ContextMenuID::ColorReset, KTr("Generic.Reset"));
					item->Enable(focusedMod->HasColor() || isMultipleSelection);
				}
			}
			contextMenu.AddSeparator();

			// Other items
			{
				KxMenuItem* item = contextMenu.AddItem(ContextMenuID::ModOpenLocation, KTr("ModManager.Menu.OpenModFilesLocation"));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FolderOpen));
				item->Enable(!isMultipleSelection && !isPriorityGroup);
			}
			{
				KxMenuItem* item = contextMenu.AddItem(ContextMenuID::ModChangeLocation, KTr("ModManager.Menu.ChangeModFilesLocation"));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FolderArrow));
				item->Enable(!isMultipleSelection && isNormalMod && !isVFSActive);
			}
			if (!isMultipleSelection && isLinkedMod && isNormalMod)
			{
				KxMenuItem* item = contextMenu.AddItem(ContextMenuID::ModRevertLocation, KTr("ModManager.Menu.RevertModFilesLocation"));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FolderArrow));
			}
			{
				KxMenuItem* item = contextMenu.AddItem(ContextMenuID::ModProperties, KTr("ModManager.Menu.Properties"));
				item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::InformationFrame));
				item->Enable(!isMultipleSelection && isNormalMod);
			}
		}

		// Refresh
		{
			KxMenuItem* item = contextMenu.AddItem(KTr(KxID_REFRESH));
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::ArrowCircleDouble));
			item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
			{
				IModManager::GetInstance()->Load();
				IModManager::GetInstance()->ResortMods();
				ScheduleReload();
			});
		}
	}

	void Workspace::SelectMod(const IGameMod* entry)
	{
		m_DisplayModel->SelectMod(entry);
	}
	void Workspace::ProcessSelection(IGameMod* entry)
	{
		DisplayModInfo(entry);
	}
	void Workspace::HighlightMod(const IGameMod* mod)
	{
		m_DisplayModel->GetView()->UnselectAll();
		m_DisplayModel->SelectMod(mod);
	}
	void Workspace::ReloadView()
	{
		m_DisplayModel->LoadView();
		ProcessSelection();
	}

	void Workspace::OnModsContextMenu(const IGameMod::RefVector& selectedMods, IGameMod* focusedMod)
	{
		// Get mouse position before doing anything else,
		// as mouse pointer can move before displaying showing the menu.
		wxPoint mousePos = wxGetMousePosition();

		// Create menu
		KxMenu contextMenu;
		CreateViewContextMenu(contextMenu, selectedMods, focusedMod);

		// Mod network menu for these mods
		for (auto& modNetwork: INetworkManager::GetInstance()->GetModNetworks())
		{
			modNetwork->OnModListMenu(contextMenu, selectedMods, focusedMod);
		}

		// Show menu
		const ContextMenuID menuID = (ContextMenuID)contextMenu.Show(nullptr, mousePos);
		switch (menuID)
		{
			// Mod
			case ContextMenuID::ModOpenLocation:
			{
				wxString location = focusedMod->IsInstalled() ? focusedMod->GetModFilesDir() : focusedMod->GetRootDir();
				KxShell::Execute(GetMainWindow(), location, "open");
				break;
			}
			case ContextMenuID::ModChangeLocation:
			{
				KxFileBrowseDialog dialog(GetMainWindow(), KxID_NONE, KxFBD_OPEN_FOLDER);
				if (dialog.ShowModal() == KxID_OK)
				{
					if (!focusedMod->IsInstalled() || KxShell::FileOperationEx(KxFOF_MOVE, focusedMod->GetModFilesDir() + "\\*", dialog.GetResult(), this, true, false, false, true))
					{
						KxFile(focusedMod->GetModFilesDir()).RemoveFolder(true);
						focusedMod->LinkLocation(dialog.GetResult());
						focusedMod->Save();

						ModEvent(Events::ModFilesChanged, *focusedMod).Send();
						ModEvent(Events::ModChanged, *focusedMod).Send();
					}
				}
				break;
			}
			case ContextMenuID::ModRevertLocation:
			{
				if (!focusedMod->IsInstalled() || KxShell::FileOperationEx(KxFOF_MOVE, focusedMod->GetModFilesDir() + "\\*", focusedMod->GetDefaultModFilesDir(), this, true, false, false, true))
				{
					// Remove file folder if it's empty
					KxFile(focusedMod->GetModFilesDir()).RemoveFolder(true);

					focusedMod->LinkLocation(wxEmptyString);
					focusedMod->Save();

					ModEvent(Events::ModFilesChanged, *focusedMod).Send();
					ModEvent(Events::ModChanged, *focusedMod).Send();
				}
				break;
			}
			case ContextMenuID::ModInstall:
			{
				InstallMod(*focusedMod);
				break;
			}
			case ContextMenuID::ModUninstall:
			case ContextMenuID::ModErase:
			{
				UninstallMod(*focusedMod, menuID == ContextMenuID::ModErase);
				break;
			}
			case ContextMenuID::ModImageShow:
			{
				KImageViewerDialog dialog(this);

				KImageViewerEvent event(wxEVT_NULL, focusedMod->GetImageFile());
				dialog.Navigate(event);
				dialog.ShowModal();
				break;
			}
			case ContextMenuID::ModImageAssign:
			{
				IGameModWithImage* withImage = nullptr;
				if (focusedMod->QueryInterface(withImage))
				{
					KxFileBrowseDialog dialog(GetMainWindow(), KxID_NONE, KxFBD_OPEN);
					dialog.AddFilter(KxString::Join(IScreenshotsGallery::GetSupportedExtensions(), ";"), KTr("FileFilter.Images"));
					if (dialog.ShowModal() == KxID_OK)
					{
						KxFile(dialog.GetResult()).CopyFile(focusedMod->GetImageFile(), true);
						withImage->ResetBitmap();
						withImage->ResetNoBitmap();
						focusedMod->Save();

						ModEvent(Events::ModChanged, *focusedMod).Send();
					}
				}
				break;
			}
			case ContextMenuID::ModEditDescription:
			{
				wxString oldDescription = focusedMod->GetDescription();
				UI::TextEditDialog dialog(GetMainWindow());
				dialog.SetText(oldDescription);

				if (dialog.ShowModal() == KxID_OK && dialog.IsModified())
				{
					focusedMod->SetDescription(dialog.GetText());
					focusedMod->Save();

					ModEvent event(Events::ModChanged, *focusedMod);
					ProcessEvent(event);
				}
				break;
			}
			case ContextMenuID::ModEditTags:
			{
				BasicGameMod tempEntry;
				tempEntry.GetTagStore() = focusedMod->GetTagStore();
				tempEntry.SetPriorityGroupTag(focusedMod->GetPriorityGroupTag());

				ModTagManager::SelectorDialog dialog(GetMainWindow(), KTr("ModManager.TagsDialog"));
				dialog.SetDataVector(&tempEntry.GetTagStore(), &tempEntry);
				dialog.ShowModal();
				if (dialog.IsModified())
				{
					dialog.ApplyChangesToMod();
					bool hasSelection = DoForAllSelectedItems(selectedMods, [&tempEntry](IGameMod& entry)
					{
						entry.GetTagStore() = tempEntry.GetTagStore();
						entry.SetPriorityGroupTag(tempEntry.GetPriorityGroupTag());
						entry.Save();

						ModEvent(Events::ModChanged, entry).Send();
						return true;
					});
					if (hasSelection)
					{
						ReloadView();
					}
				}
				break;
			}
			case ContextMenuID::ModEditSources:
			{
				ModSource::StoreDialog dialog(GetMainWindow(), focusedMod->GetModSourceStore());
				dialog.ShowModal();
				if (dialog.IsModified())
				{
					focusedMod->Save();
					ModEvent(Events::ModChanged, *focusedMod).Send();
				}
				break;
			}
			case ContextMenuID::ModChangeID:
			{
				if (ShowChangeModIDDialog(focusedMod))
				{
					m_DisplayModel->UpdateUI();
				}
				break;
			}
			case ContextMenuID::ModExploreFiles:
			{
				KModFilesExplorerDialog dialog(this, *focusedMod);
				dialog.ShowModal();
				break;
			}
			case ContextMenuID::ModShowCollisions:
			{
				new KModCollisionViewerModelDialog(this, focusedMod);
				break;
			}
			case ContextMenuID::ModProperties:
			{
				KxShell::Execute(GetMainWindow(), focusedMod->GetModFilesDir(), "properties");
				break;
			}

			// Color menu
			case ContextMenuID::ColorAssign:
			{
				wxColourData colorData;
				colorData.SetColour(focusedMod->GetColor());
				colorData.SetChooseFull(true);
				colorData.SetChooseAlpha(true);

				wxColourDialog dialog(this, &colorData);
				if (dialog.ShowModal() == wxID_OK)
				{
					DoForAllSelectedItems(selectedMods, [&dialog](IGameMod& entry)
					{
						entry.SetColor(dialog.GetColourData().GetColour());
						entry.Save();

						ModEvent(Events::ModChanged, entry).Send();
						return true;
					});
					m_DisplayModel->UpdateUI();
				}
				break;
			}
			case ContextMenuID::ColorReset:
			{
				DoForAllSelectedItems(selectedMods, [](IGameMod& entry)
				{
					entry.SetColor(KxColor());
					entry.Save();

					ModEvent(Events::ModChanged, entry).Send();
					return true;
				});
				m_DisplayModel->UpdateUI();
				break;
			}
		};
	}
	void Workspace::UpdateModListContent()
	{
		m_ToolBar_Profiles->Clear();
		int selectIndex = 0;
		for (const auto& profile: IGameInstance::GetActive()->GetProfiles())
		{
			int index = m_ToolBar_Profiles->Append(profile->GetID());
			if (profile->GetID() == IGameInstance::GetActiveProfileID())
			{
				selectIndex = index;
			}
		}
		m_ToolBar_Profiles->SetSelection(selectIndex);
	}

	bool Workspace::IsAnyChangeAllowed() const
	{
		return IsMovingModsAllowed();
	}
	bool Workspace::IsMovingModsAllowed() const
	{
		return m_DisplayModel->GetDisplayMode() == DisplayModelType::Connector && IsChangingModsAllowed();
	}
	bool Workspace::IsChangingModsAllowed() const
	{
		return !IModManager::GetInstance()->GetFileSystem().IsEnabled();
	}
}
