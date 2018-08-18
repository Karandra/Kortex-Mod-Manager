#include "stdafx.h"
#include "KPackageCreatorWorkspace.h"
#include "KPackageCreatorController.h"
#include "KPackageCreatorPageBase.h"
#include "KPackageCreatorPageInfo.h"
#include "KPackageCreatorPageFileData.h"
#include "KPackageCreatorPageInterface.h"
#include "KPackageCreatorPageRequirements.h"
#include "KPackageCreatorPageComponents.h"
#include "KPackageCreatorFOModIEDialog.h"
#include "PackageProject/KPackageProjectDefs.h"
#include "PackageProject/KPackageProjectSerializerKMP.h"
#include "PackageProject/KPackageProjectSerializerFOMod.h"
#include "PackageManager/KModPackage.h"
#include "KThemeManager.h"
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxProgressDialog.h>
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxFileStream.h>
#pragma warning (disable: 4302)

KxSingletonPtr_Define(KPackageCreatorWorkspace);

KPackageCreatorWorkspace::KPackageCreatorWorkspace(KMainWindow* mainWindow)
	:KWorkspace(mainWindow)
{
	CreateItemInManagersMenu();
	m_MainSizer = new wxBoxSizer(wxVERTICAL);
}
KPackageCreatorWorkspace::~KPackageCreatorWorkspace()
{
}
bool KPackageCreatorWorkspace::OnCreateWorkspace()
{
	KThemeManager::Get().ProcessWindow(static_cast<wxWindow*>(this));

	wxBoxSizer* mainPaneSizer = new wxBoxSizer(wxHORIZONTAL);
	m_MainPane = new KxPanel(this, KxID_NONE);
	m_MainPane->SetSizer(mainPaneSizer);

	m_MainSizer->Add(m_MainPane, 1, wxEXPAND);
	KThemeManager::Get().ProcessWindow(m_MainPane);

	CreatePagesListView();
	CreatePagesView();
	
	mainPaneSizer->Add(m_PagesList, 0, wxEXPAND);
	mainPaneSizer->Add(m_PagesBookPane, 1, wxEXPAND|wxLEFT, KLC_HORIZONTAL_SPACING);

	// Create controller
	m_Controller = new KPackageCreatorController(this);
	SetWorkspaceController(m_Controller);

	// Create pages
	m_PageInfo = AddPage(new KPackageCreatorPageInfo(this, m_Controller));
	m_PageFileData = AddPage(new KPackageCreatorPageFileData(this, m_Controller));
	m_PageInterface = AddPage(new KPackageCreatorPageInterface(this, m_Controller));
	m_PageRequirements = AddPage(new KPackageCreatorPageRequirements(this, m_Controller));
	m_PageComponents = AddPage(new KPackageCreatorPageComponents(this, m_Controller));

	// Open first page (Info)
	SwitchToPage(0);
	m_PagesList->GetFirstItem().SetSelection();
	m_PagesList->SetFocus();

	// Create empty project
	m_Controller->NewProject();
	return true;
}

bool KPackageCreatorWorkspace::OnOpenWorkspace()
{
	RefreshWindowTitleForProject();
	return true;
}
bool KPackageCreatorWorkspace::OnCloseWorkspace()
{
	if (m_Controller->AskForSave() == KxID_OK)
	{
		return true;
	}
	return false;
}
void KPackageCreatorWorkspace::OnReloadWorkspace()
{
}

void KPackageCreatorWorkspace::CreateMenuBar(wxSizer* sizer)
{
	m_MenuBar = new KxAuiToolBar(m_PagesBookPane, KxID_NONE, KxAuiToolBar::DefaultStyle|wxAUI_TB_PLAIN_BACKGROUND|wxAUI_TB_TEXT);
	m_MenuBar->SetBackgroundColour(GetBackgroundColour());
	m_MenuBar->SetBorderColor(KThemeManager::Get().GetColor(KTMC_BORDER));
	m_MenuBar->SetToolPacking(0);
	m_MenuBar->SetToolSeparation(0);
	m_MenuBar->SetMargins(0, 0, 0, 2);
	KThemeManager::Get().ProcessWindow(static_cast<wxWindow*>(m_MenuBar));

	m_MenuBar_Project = m_MenuBar->AddTool(T("PackageCreator.MenuBar.Project"), wxNullBitmap);
	m_MenuBar_Import = m_MenuBar->AddTool(T("PackageCreator.MenuBar.Import"), wxNullBitmap);
	m_MenuBar_Build = m_MenuBar->AddTool(T("PackageCreator.MenuBar.Build"), wxNullBitmap);

	m_MenuBar->Realize();
	sizer->Add(m_MenuBar, 0, wxEXPAND);

	CreateProjectMenu();
	CreateImportMenu();
	CreateBuildMenu();
}
void KPackageCreatorWorkspace::CreateProjectMenu()
{
	KxMenu* menu = new KxMenu();
	m_MenuBar_Project->AssignDropdownMenu(menu);
	m_MenuBar_Project->SetOptionEnabled(KxAUI_TBITEM_OPTION_LCLICK_MENU);
	KxMenuItem* item = NULL;

	item = menu->Add(new KxMenuItem(KxID_NEW, T("PackageCreator.MenuProject.New")));
	item->SetBitmap(KGetBitmap(KIMG_DOCUMENT_NEW));
	item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnNewProject, this);

	item = menu->Add(new KxMenuItem(KxID_OPEN, T("PackageCreator.MenuProject.Open")));
	item->SetBitmap(KGetBitmap(KIMG_DOCUMENT));
	item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnOpenProject, this);
	menu->AddSeparator();

	item = menu->Add(new KxMenuItem(KxID_SAVE, T("PackageCreator.MenuProject.Save")));
	item->SetBitmap(KGetBitmap(KIMG_DISK));
	item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnSaveProject, this);

	item = menu->Add(new KxMenuItem(KxID_SAVEAS, T("PackageCreator.MenuProject.SaveAs")));
	item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnSaveProject, this);

	item = menu->Add(new KxMenuItem(KxID_HIGHEST + KPP_PACCKAGE_FOMOD_XML, T("PackageCreator.MenuProject.SaveAsFOMod")));
	item->SetBitmap(KGetBitmap(KIMG_DOCUMENT_EXPORT));
	item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnExportProject, this);

	item = menu->Add(new KxMenuItem(KxID_HIGHEST + KPP_PACCKAGE_NATIVE, T("PackageCreator.MenuProject.SaveAsKMP")));
	item->SetBitmap(KGetBitmap(KIMG_DOCUMENT_EXPORT));
	item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnExportProject, this);
}
void KPackageCreatorWorkspace::CreateImportMenu()
{
	KxMenu* menu = new KxMenu();
	m_MenuBar_Import->AssignDropdownMenu(menu);
	m_MenuBar_Import->SetOptionEnabled(KxAUI_TBITEM_OPTION_LCLICK_MENU);
	KxMenuItem* item = NULL;

	item = menu->Add(new KxMenuItem(KxID_HIGHEST + 0, T("PackageCreator.MenuImport.FOModXML")));
	item->SetBitmap(KGetBitmap(KIMG_DOCUMENT_IMPORT));
	item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnImportProject, this);

	item = menu->Add(new KxMenuItem(KxID_HIGHEST + 1, T("PackageCreator.MenuImport.Package")));
	item->SetBitmap(KGetBitmap(KIMG_DOCUMENT_IMPORT));
	item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnImportProject, this);
}
void KPackageCreatorWorkspace::CreateBuildMenu()
{
	KxMenu* menu = new KxMenu();
	m_MenuBar_Build->AssignDropdownMenu(menu);
	m_MenuBar_Build->SetOptionEnabled(KxAUI_TBITEM_OPTION_LCLICK_MENU);
	KxMenuItem* item = NULL;

	item = menu->Add(new KxMenuItem(T("PackageCreator.MenuBuild.Build")));
	item->SetBitmap(KGetBitmap(KIMG_COMPILE));
	item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnBuildProject, this);

	item = menu->Add(new KxMenuItem(T("PackageCreator.MenuBuild.Preview")));
	item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnBuildProjectPreview, this);
}

void KPackageCreatorWorkspace::CreatePagesListView()
{
	m_PagesList = new KxTreeList(m_MainPane, KxID_NONE, KxTreeList::DefaultStyle|wxTL_NO_HEADER);
	m_PagesList->GetDataView()->ToggleWindowStyle(wxBORDER_NONE);
	m_PagesList->SetImageList(const_cast<KxImageList*>(KGetImageList()));
	m_PagesList->SetRowHeight(36);
	m_PagesList->SetMinSize(wxSize(180, wxDefaultCoord));
	m_PagesList->SetMaxSize(wxSize(180, wxDefaultCoord));

	m_PagesList->AddColumn(wxEmptyString);
	KThemeManager::Get().ProcessWindow(m_PagesList);

	m_PagesList->Bind(KxEVT_TREELIST_SELECTION_CHANGED, &KPackageCreatorWorkspace::OnSwitchPage, this);
}
void KPackageCreatorWorkspace::CreatePagesView()
{
	// Main workspace area
	wxBoxSizer* pagesBookPaneSizer = new wxBoxSizer(wxVERTICAL);
	m_PagesBookPane = new KxPanel(m_MainPane, KxID_NONE);
	m_PagesBookPane->SetSizer(pagesBookPaneSizer);

	// Menu bar
	CreateMenuBar(pagesBookPaneSizer);

	// Pages book
	m_PagesBook = new wxSimplebook(m_PagesBookPane, KxID_NONE);
	pagesBookPaneSizer->Add(m_PagesBook, 1, wxEXPAND);
	KThemeManager::Get().ProcessWindow(m_PagesBook);
}

void KPackageCreatorWorkspace::AddPageBase(KPackageCreatorPageBase* page, bool select)
{
	if (page->OnCreateWorkspaceInternal())
	{
		m_PagesBook->AddPage(page, page->GetPageName(), select, page->GetImageID());

		KxTreeListItem pageItem = m_PagesList->GetRoot().Add(page->GetPageName());
		pageItem.SetImage(page->GetImageID());

		if (select)
		{
			SwitchToPage(m_PagesBook->GetPageCount() - 1);
		}
	}
}
void KPackageCreatorWorkspace::OnSwitchPage(wxTreeListEvent& event)
{
	KxTreeListItem item(*m_PagesList, event.GetItem());
	if (item.IsOK())
	{
		SwitchToPage((size_t)item.GetIndexWithinParent());
	}
}
void KPackageCreatorWorkspace::DoLoadAllPages()
{
	m_PageInfo->OnLoadProject(m_Controller->GetProject()->GetInfo());
	m_PageFileData->OnLoadProject(m_Controller->GetProject()->GetFileData());
	m_PageInterface->OnLoadProject(m_Controller->GetProject()->GetInterface());
	m_PageRequirements->OnLoadProject(m_Controller->GetProject()->GetRequirements());
	m_PageComponents->OnLoadProject(m_Controller->GetProject()->GetComponents());
}

wxString KPackageCreatorWorkspace::GetID() const
{
	return "KPackageCreatorWorkspace";
}
wxString KPackageCreatorWorkspace::GetName() const
{
	return T("ToolBar.PackageCreator");
}

KPackageCreatorPageBase* KPackageCreatorWorkspace::GetCurrentPage() const
{
	return static_cast<KPackageCreatorPageBase*>(m_PagesBook->GetCurrentPage());
}
bool KPackageCreatorWorkspace::SwitchToPage(size_t index)
{
	if (index >= 0 && index < m_PagesBook->GetPageCount())
	{
		bool allow = true;
		KPackageCreatorPageBase* current = GetCurrentPage();
		if (current)
		{
			allow = current->OnCloseWorkspaceInternal();
		}

		KPackageCreatorPageBase* next = static_cast<KPackageCreatorPageBase*>(m_PagesBook->GetPage(index));
		if (allow && next && next->OnOpenWorkspaceInternal())
		{
			m_PagesBook->ChangeSelection(index);
			return true;
		}
	}
	return false;
}
KPackageProject& KPackageCreatorWorkspace::ImportProjectFromPackage(const wxString& path)
{
	m_Controller->ImportProjectFromPackage(path);
	return *m_Controller->GetProject();
}
KPackageProject& KPackageCreatorWorkspace::CreateProjectFromModEntry(const KModEntry& modEntry)
{
	m_Controller->CreateProjectFromModEntry(modEntry);
	return *m_Controller->GetProject();
}

void KPackageCreatorWorkspace::RefreshWindowTitleForProject()
{
	KPackageCreatorPageBase* current = GetCurrentPage();
	if (current)
	{
		current->RefreshWindowTitle();
	}
}
void KPackageCreatorWorkspace::OnNewProject(KxMenuEvent& event)
{
	if (m_Controller->AskForSave() == KxID_OK)
	{
		m_Controller->NewProject();
		DoLoadAllPages();
	}
}
void KPackageCreatorWorkspace::OnOpenProject(KxMenuEvent& event)
{
	wxWindowUpdateLocker lock(this);
	if (m_Controller->AskForSave() == KxID_OK)
	{
		KxFileBrowseDialog dialog(GetMainWindow(), KxID_NONE, KxFBD_OPEN);
		dialog.AddFilter("*.kmpproj", T("FileFilter.ModProject"));
		dialog.AddFilter("*", T("FileFilter.AllFiles"));

		if (dialog.ShowModal() == KxID_OK)
		{
			m_Controller->OpenProject(dialog.GetResult());
			DoLoadAllPages();
		}
	}
}
void KPackageCreatorWorkspace::OnSaveProject(KxMenuEvent& event)
{
	if (event.GetItem()->GetId() == KxID_SAVEAS || !m_Controller->HasProjectFilePath() || !wxFileName(m_Controller->GetProjectFilePath()).Exists(wxFILE_EXISTS_REGULAR))
	{
		KxFileBrowseDialog dialog(GetMainWindow(), KxID_NONE, KxFBD_SAVE);
		dialog.SetDefaultExtension("kmpproj");
		dialog.SetFileName(m_Controller->GetProjectName());
		dialog.AddFilter("*.kmpproj", T("FileFilter.ModProject"));
		dialog.AddFilter("*", T("FileFilter.AllFiles"));

		if (dialog.ShowModal() == KxID_OK)
		{
			m_Controller->SaveProject(dialog.GetResult());
		}
	}
	else
	{
		m_Controller->SaveProject();
	}
}
void KPackageCreatorWorkspace::OnImportProject(KxMenuEvent& event)
{
	wxWindowUpdateLocker lock(this);
	if (m_Controller->AskForSave() == KxID_OK)
	{
		KxMenuItem* item = event.GetItem();

		KPPPackageType type = (KPPPackageType)(item->GetId() - KxID_HIGHEST);
		switch (type)
		{
			case 0:
			{
				KPackageCreatorFOModIEDialog dialog(GetMainWindow(), false);
				if (dialog.ShowModal() == KxID_OK)
				{
					wxString info = KxTextFile::ReadToString(dialog.GetInfoFile());
					wxString sModuleConfig = KxTextFile::ReadToString(dialog.GetModuleConfigFile());

					KPackageProjectSerializerFOMod tSerailizer(info, sModuleConfig, dialog.GetProjectFolder());
					m_Controller->ImportProject(tSerailizer);
				}
				break;
			}
			case 1:
			{
				KxFileBrowseDialog dialog(GetMainWindow(), KxID_NONE, KxFBD_OPEN);
				dialog.AddFilter("*.kmp;*.smi;*.7z;*.zip;*.fomod", T("FileFilter.AllSupportedFormats"));
				dialog.AddFilter("*", T("FileFilter.AllFiles"));
				if (dialog.ShowModal() == KxID_OK)
				{
					ImportProjectFromPackage(dialog.GetResult());
				}
				break;
			}
		};
	}
}
void KPackageCreatorWorkspace::OnExportProject(KxMenuEvent& event)
{
	KPPPackageType type = (KPPPackageType)(event.GetItem()->GetId() - KxID_HIGHEST);
	switch (type)
	{
		case KPP_PACCKAGE_NATIVE:
		{
			KxFileBrowseDialog dialog(GetMainWindow(), KxID_NONE, KxFBD_SAVE);
			dialog.SetDefaultExtension("xml");
			dialog.SetFileName(m_Controller->GetProjectName());
			dialog.AddFilter("*.xml", T("FileFilter.XML"));
			dialog.AddFilter("*", T("FileFilter.AllFiles"));

			if (dialog.ShowModal() == KxID_OK)
			{
				KPackageProjectSerializerKMP serializer(false);
				m_Controller->ExportProject(serializer);
				KxTextFile::WriteToFile(dialog.GetResult(), serializer.GetData());
			}
			break;
		}
		case KPP_PACCKAGE_FOMOD_XML:
		{
			KPackageCreatorFOModIEDialog dialog(GetMainWindow(), true);
			if (dialog.ShowModal() == KxID_OK)
			{
				KPackageProjectSerializerFOMod serializer(dialog.GetProjectFolder());
				serializer.ExportToNativeFormat(true);
				m_Controller->ExportProject(serializer);

				if (!dialog.GetInfoFile().IsEmpty())
				{
					KxTextFile::WriteToFile(dialog.GetInfoFile(), serializer.GetInfoXML());
				}
				if (!dialog.GetModuleConfigFile().IsEmpty())
				{
					KxTextFile::WriteToFile(dialog.GetModuleConfigFile(), serializer.GetModuleConfigXML());
				}
			}
			break;
		}
	};
}
void KPackageCreatorWorkspace::OnBuildProject(KxMenuEvent& event)
{
	m_Controller->BuildProject();
}
void KPackageCreatorWorkspace::OnBuildProjectPreview(KxMenuEvent& event)
{
	m_Controller->BuildProject(true);
}
