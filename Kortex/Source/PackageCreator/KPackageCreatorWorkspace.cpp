#include "stdafx.h"
#include <Kortex/Application.hpp>
#include <Kortex/ModManager.hpp>
#include "KPackageCreatorWorkspace.h"
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
#include "ModPackages/ModPackage.h"
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxProgressDialog.h>
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxFileStream.h>

namespace Kortex::PackageDesigner
{
	void WorkspaceContainer::Create(wxWindow* listParent, wxWindow* viewParent)
	{
		// List
		m_PagesList = new KxTreeList(listParent, KxID_NONE, KxTreeList::DefaultStyle|wxTL_NO_HEADER);
		m_PagesList->GetDataView()->ToggleWindowStyle(wxBORDER_NONE);
		m_PagesList->SetImageList(&ImageProvider::GetImageList());
		m_PagesList->SetRowHeight(36);
		m_PagesList->SetMinSize(wxSize(180, wxDefaultCoord));
		m_PagesList->SetMaxSize(wxSize(180, wxDefaultCoord));

		m_PagesList->AddColumn(wxEmptyString);
		m_PagesList->Bind(wxEVT_TREELIST_SELECTION_CHANGED, &WorkspaceContainer::OnPageSelected, this);
		IThemeManager::GetActive().ProcessWindow(m_PagesList);

		// View
		m_BookCtrl = new KxSimplebook(viewParent, KxID_NONE);
		IThemeManager::GetActive().ProcessWindow(m_BookCtrl);
	}
	void WorkspaceContainer::OnPageSelected(wxTreeListEvent& event)
	{
		if (KxTreeListItem item(*m_PagesList, event.GetItem()); item.IsOK())
		{
			if (IWorkspace* workspace = GetWorkspaceByIndex(item.GetIndexWithinParent()))
			{
				workspace->SwitchHere();
			}
		}
	}

	void WorkspaceContainer::ShowWorkspace(IWorkspace& workspace)
	{
		if (auto index = GetWorkspaceIndex(workspace))
		{
			WorkspaceBookContainer::AddWorkspace(workspace);

			KxTreeListItem item = m_PagesList->GetRoot().GetNthChild(*index);
			item.SetSelection();
		}
	}
	void WorkspaceContainer::HideWorkspace(IWorkspace& workspace)
	{
		return WorkspaceBookContainer::HideWorkspace(workspace);
	}
}

namespace Kortex::PackageDesigner
{
	bool KPackageCreatorWorkspace::OnCreateWorkspace()
	{
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
		SetSizer(m_MainSizer);

		wxBoxSizer* mainPaneSizer = new wxBoxSizer(wxHORIZONTAL);
		m_MainPane = new KxPanel(this, KxID_NONE);
		m_MainPane->SetSizer(mainPaneSizer);

		m_MainSizer->Add(m_MainPane, 1, wxEXPAND);
		IThemeManager::GetActive().ProcessWindow(m_MainPane);
		CreatePagesView();

		mainPaneSizer->Add(&m_PagesContainer.GetListWindow(), 0, wxEXPAND);
		mainPaneSizer->Add(m_PagesBookPane, 1, wxEXPAND|wxLEFT, KLC_HORIZONTAL_SPACING);

		// Create pages
		m_PageInfo = &m_PagesContainer.MakeWorkspace<KPackageCreatorPageInfo>(*this, m_Controller);
		m_PageFileData = &m_PagesContainer.MakeWorkspace<KPackageCreatorPageFileData>(*this, m_Controller);
		m_PageInterface = &m_PagesContainer.MakeWorkspace<KPackageCreatorPageInterface>(*this, m_Controller);
		m_PageRequirements = &m_PagesContainer.MakeWorkspace<KPackageCreatorPageRequirements>(*this, m_Controller);
		m_PageComponents = &m_PagesContainer.MakeWorkspace<KPackageCreatorPageComponents>(*this, m_Controller);

		// Open first page (Info)
		m_PagesContainer.SwitchWorkspace(*m_PageInfo);

		// Create empty project
		m_Controller.NewProject();
		return true;
	}
	bool KPackageCreatorWorkspace::OnOpenWorkspace()
	{
		return true;
	}
	bool KPackageCreatorWorkspace::OnCloseWorkspace()
	{
		return m_Controller.AskForSave() == KxID_OK;
	}
	void KPackageCreatorWorkspace::OnReloadWorkspace()
	{
	}

	void KPackageCreatorWorkspace::CreateMenuBar(wxSizer* sizer)
	{
		m_MenuBar = new KxAuiToolBar(m_PagesBookPane, KxID_NONE, KxAuiToolBar::DefaultStyle|wxAUI_TB_PLAIN_BACKGROUND|wxAUI_TB_TEXT);
		m_MenuBar->SetBackgroundColour(GetBackgroundColour());
		m_MenuBar->SetBorderColor(IThemeManager::GetActive().GetColor(IThemeManager::ColorIndex::Border));
		m_MenuBar->SetToolPacking(0);
		m_MenuBar->SetToolSeparation(0);
		m_MenuBar->SetMargins(0, 0, 0, 2);
		IThemeManager::GetActive().ProcessWindow(static_cast<wxWindow*>(m_MenuBar));

		m_MenuBar_Project = m_MenuBar->AddTool(KTr("PackageCreator.MenuBar.Project"), wxNullBitmap);
		m_MenuBar_Import = m_MenuBar->AddTool(KTr("PackageCreator.MenuBar.Import"), wxNullBitmap);
		m_MenuBar_Build = m_MenuBar->AddTool(KTr("PackageCreator.MenuBar.Build"), wxNullBitmap);

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
		KxMenuItem* item = nullptr;

		item = menu->Add(new KxMenuItem(KxID_NEW, KTr("PackageCreator.MenuProject.New")));
		item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::DocumentNew));
		item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnNewProject, this);

		item = menu->Add(new KxMenuItem(KxID_OPEN, KTr("PackageCreator.MenuProject.Open")));
		item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Document));
		item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnOpenProject, this);
		menu->AddSeparator();

		item = menu->Add(new KxMenuItem(KxID_SAVE, KTr("PackageCreator.MenuProject.Save")));
		item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Disk));
		item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnSaveProject, this);

		item = menu->Add(new KxMenuItem(KxID_SAVEAS, KTr("PackageCreator.MenuProject.SaveAs")));
		item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnSaveProject, this);

		item = menu->Add(new KxMenuItem(KxID_HIGHEST + KPP_PACCKAGE_FOMOD_XML, KTr("PackageCreator.MenuProject.SaveAsFOMod")));
		item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::DocumentExport));
		item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnExportProject, this);

		item = menu->Add(new KxMenuItem(KxID_HIGHEST + KPP_PACCKAGE_NATIVE, KTr("PackageCreator.MenuProject.SaveAsKMP")));
		item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::DocumentExport));
		item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnExportProject, this);
	}
	void KPackageCreatorWorkspace::CreateImportMenu()
	{
		KxMenu* menu = new KxMenu();
		m_MenuBar_Import->AssignDropdownMenu(menu);
		m_MenuBar_Import->SetOptionEnabled(KxAUI_TBITEM_OPTION_LCLICK_MENU);
		KxMenuItem* item = nullptr;

		item = menu->Add(new KxMenuItem(KxID_HIGHEST + 0, KTr("PackageCreator.MenuImport.FOModXML")));
		item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::DocumentImport));
		item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnImportProject, this);

		item = menu->Add(new KxMenuItem(KxID_HIGHEST + 1, KTr("PackageCreator.MenuImport.Package")));
		item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::DocumentImport));
		item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnImportProject, this);
	}
	void KPackageCreatorWorkspace::CreateBuildMenu()
	{
		KxMenu* menu = new KxMenu();
		m_MenuBar_Build->AssignDropdownMenu(menu);
		m_MenuBar_Build->SetOptionEnabled(KxAUI_TBITEM_OPTION_LCLICK_MENU);
		KxMenuItem* item = nullptr;

		item = menu->Add(new KxMenuItem(KTr("PackageCreator.MenuBuild.Build")));
		item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::Compile));
		item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnBuildProject, this);

		item = menu->Add(new KxMenuItem(KTr("PackageCreator.MenuBuild.Preview")));
		item->Bind(KxEVT_MENU_SELECT, &KPackageCreatorWorkspace::OnBuildProjectPreview, this);
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
		m_PagesContainer.Create(m_MainPane, m_PagesBookPane);
		pagesBookPaneSizer->Add(&m_PagesContainer.GetWindow(), 1, wxEXPAND);
	}

	void KPackageCreatorWorkspace::DoLoadAllPages()
	{
		m_PageInfo->OnLoadProject(m_Controller.GetProject()->GetInfo());
		m_PageFileData->OnLoadProject(m_Controller.GetProject()->GetFileData());
		m_PageInterface->OnLoadProject(m_Controller.GetProject()->GetInterface());
		m_PageRequirements->OnLoadProject(m_Controller.GetProject()->GetRequirements());
		m_PageComponents->OnLoadProject(m_Controller.GetProject()->GetComponents());
	}
	void KPackageCreatorWorkspace::OnNewProject(KxMenuEvent& event)
	{
		if (m_Controller.AskForSave() == KxID_OK)
		{
			m_Controller.NewProject();
			DoLoadAllPages();
		}
	}
	void KPackageCreatorWorkspace::OnOpenProject(KxMenuEvent& event)
	{
		wxWindowUpdateLocker lock(this);

		if (m_Controller.AskForSave() == KxID_OK)
		{
			KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_OPEN);
			dialog.AddFilter("*.kmpproj", KTr("FileFilter.ModProject"));
			dialog.AddFilter("*", KTr("FileFilter.AllFiles"));

			if (dialog.ShowModal() == KxID_OK)
			{
				m_Controller.OpenProject(dialog.GetResult());
				DoLoadAllPages();
			}
		}
	}
	void KPackageCreatorWorkspace::OnSaveProject(KxMenuEvent& event)
	{
		if (event.GetItem()->GetId() == KxID_SAVEAS || !m_Controller.HasProjectFilePath() || !wxFileName(m_Controller.GetProjectFilePath()).Exists(wxFILE_EXISTS_REGULAR))
		{
			KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_SAVE);
			dialog.SetDefaultExtension("kmpproj");
			dialog.SetFileName(m_Controller.GetProjectName());
			dialog.AddFilter("*.kmpproj", KTr("FileFilter.ModProject"));
			dialog.AddFilter("*", KTr("FileFilter.AllFiles"));

			if (dialog.ShowModal() == KxID_OK)
			{
				m_Controller.SaveProject(dialog.GetResult());
			}
		}
		else
		{
			m_Controller.SaveProject();
		}
	}
	void KPackageCreatorWorkspace::OnImportProject(KxMenuEvent& event)
	{
		wxWindowUpdateLocker lock(this);
		if (m_Controller.AskForSave() == KxID_OK)
		{
			KxMenuItem* item = event.GetItem();

			KPPPackageType type = (KPPPackageType)(item->GetId() - KxID_HIGHEST);
			switch (type)
			{
				case 0:
				{
					KPackageCreatorFOModIEDialog dialog(this, false);
					if (dialog.ShowModal() == KxID_OK)
					{
						wxString info = KxTextFile::ReadToString(dialog.GetInfoFile());
						wxString sModuleConfig = KxTextFile::ReadToString(dialog.GetModuleConfigFile());

						KPackageProjectSerializerFOMod tSerailizer(info, sModuleConfig, dialog.GetProjectFolder());
						m_Controller.ImportProject(tSerailizer);
					}
					break;
				}
				case 1:
				{
					KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_OPEN);
					dialog.AddFilter("*.kmp;*.smi;*.7z;*.zip;*.fomod", KTr("FileFilter.AllSupportedFormats"));
					dialog.AddFilter("*", KTr("FileFilter.AllFiles"));
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
				KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_SAVE);
				dialog.SetDefaultExtension("xml");
				dialog.SetFileName(m_Controller.GetProjectName());
				dialog.AddFilter("*.xml", KTr("FileFilter.XML"));
				dialog.AddFilter("*", KTr("FileFilter.AllFiles"));

				if (dialog.ShowModal() == KxID_OK)
				{
					KPackageProjectSerializerKMP serializer(false);
					m_Controller.ExportProject(serializer);
					KxTextFile::WriteToFile(dialog.GetResult(), serializer.GetData());
				}
				break;
			}
			case KPP_PACCKAGE_FOMOD_XML:
			{
				KPackageCreatorFOModIEDialog dialog(this, true);
				if (dialog.ShowModal() == KxID_OK)
				{
					KPackageProjectSerializerFOMod serializer(dialog.GetProjectFolder());
					serializer.ExportToNativeFormat(true);
					m_Controller.ExportProject(serializer);

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
		m_Controller.BuildProject();
	}
	void KPackageCreatorWorkspace::OnBuildProjectPreview(KxMenuEvent& event)
	{
		m_Controller.BuildProject(true);
	}

	wxString KPackageCreatorWorkspace::GetID() const
	{
		return "KPackageCreatorWorkspace";
	}
	wxString KPackageCreatorWorkspace::GetName() const
	{
		return KTr("PackageManager.CreatorName");
	}

	KPackageCreatorPageBase* KPackageCreatorWorkspace::GetCurrentPage() const
	{
		return static_cast<KPackageCreatorPageBase*>(m_PagesContainer.GetCurrentWorkspace());
	}
	KPackageProject& KPackageCreatorWorkspace::ImportProjectFromPackage(const wxString& path)
	{
		EnsureCreated();

		m_Controller.ImportProjectFromPackage(path);
		return *m_Controller.GetProject();
	}
	KPackageProject& KPackageCreatorWorkspace::CreateProjectFromModEntry(const Kortex::IGameMod& modEntry)
	{
		EnsureCreated();

		m_Controller.CreateProjectFromModEntry(modEntry);
		return *m_Controller.GetProject();
	}
}
