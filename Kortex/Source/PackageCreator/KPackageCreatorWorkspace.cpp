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
#include "GameMods/ModManager/Workspace.h"
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxProgressDialog.h>
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxFileStream.h>

namespace Kortex::Application::OName
{
	KortexDefOption(Splitter);
	KortexDefOption(Pages);
}

namespace
{
	template<class T> auto GetUIOption(T&& option)
	{
		using namespace Kortex;
		using namespace Kortex::Application;

		return Application::GetAInstanceOptionOf<PackageDesigner::KPackageCreatorWorkspace>(std::forward<T>(option));
	}
}

namespace Kortex::PackageDesigner
{
	void WorkspaceContainer::Create(wxWindow* listParent, wxWindow* viewParent)
	{
		// List
		m_PagesList = new KxTreeList(listParent, KxID_NONE, KxTreeList::DefaultStyle|wxTL_NO_HEADER);
		m_PagesList->GetDataView()->ToggleWindowStyle(wxBORDER_NONE);
		m_PagesList->SetImageList(&ImageProvider::GetImageList());
		m_PagesList->SetRowHeight(m_PagesList->FromDIP(36));

		m_PagesList->AddColumn(wxEmptyString);
		m_PagesList->Bind(wxEVT_TREELIST_SELECTION_CHANGED, &WorkspaceContainer::OnPageSelected, this);
		IThemeManager::GetActive().Apply(m_PagesList);

		// View
		m_BookCtrl = new KxSimplebook(viewParent, KxID_NONE);
		IThemeManager::GetActive().Apply(m_BookCtrl);
	}
	void WorkspaceContainer::OnPageSelected(wxTreeListEvent& event)
	{
		if (KxTreeListItem item(*m_PagesList, event.GetItem()); item.IsOK())
		{
			if (auto clientData = static_cast<Application::WorkspaceClientData*>(item.GetData()))
			{
				clientData->GetWorkspace().SwitchHere();
			}
		}
	}

	void WorkspaceContainer::ShowWorkspace(IWorkspace& workspace)
	{
		if (auto index = GetWorkspaceIndex(workspace))
		{
			BookWorkspaceContainer::ShowWorkspace(workspace);

			KxTreeListItem root = m_PagesList->GetRoot();
			for (KxTreeListItem item = root.GetFirstChild(); item.IsOK(); item = item.GetNextSibling())
			{
				auto clientData = static_cast<Application::WorkspaceClientData*>(item.GetData());
				if (clientData && &clientData->GetWorkspace() == &workspace)
				{
					item.SetSelection();
					break;
				}
			}
		}
	}
	void WorkspaceContainer::HideWorkspace(IWorkspace& workspace)
	{
		return BookWorkspaceContainer::HideWorkspace(workspace);
	}

	bool WorkspaceContainer::AddWorkspace(IWorkspace& workspace)
	{
		if (Application::BookWorkspaceContainer::AddWorkspace(workspace))
		{
			KxTreeListItem item = m_PagesList->GetRoot().Add(workspace.GetName(), new Application::WorkspaceClientData(workspace));
			if (auto icon = workspace.GetIcon().TryAsInt())
			{
				// TODO: KxTreeList is broken, replace with proper control such as KxDataView2
				m_PagesList->wxTreeListCtrl::SetItemImage(item, *icon, *icon);
			}

			return true;
		}
		return false;
	}
	bool WorkspaceContainer::RemoveWorkspace(IWorkspace& workspace)
	{
		if (Application::BookWorkspaceContainer::RemoveWorkspace(workspace))
		{
			KxTreeListItem root = m_PagesList->GetRoot();
			for (KxTreeListItem item = root.GetFirstChild(); item.IsOK(); item = item.GetNextSibling())
			{
				auto clientData = static_cast<Application::WorkspaceClientData*>(item.GetData());
				if (clientData && &clientData->GetWorkspace() == &workspace)
				{
					item.Remove();
					return true;
				}
			}
		}
		return false;
	}
}

namespace Kortex::PackageDesigner
{
	bool KPackageCreatorWorkspace::OnCreateWorkspace()
	{
		m_SplitterLeftRight = new KxSplitterWindow(this, KxID_NONE);
		m_SplitterLeftRight->SetMinimumPaneSize(ImageProvider::GetImageList().GetSize().GetWidth() * 1.5);

		// Right pane
		wxBoxSizer* rightPaneSizer = new wxBoxSizer(wxVERTICAL);
		m_RightPane = new KxPanel(m_SplitterLeftRight, KxID_NONE);
		m_RightPane->SetSizer(rightPaneSizer);
		IThemeManager::GetActive().Apply(m_RightPane);

		// Left pane
		m_PagesContainer.Create(m_SplitterLeftRight, m_RightPane);
		
		// Menu bar
		CreateMenuBar(rightPaneSizer);

		// Layout
		m_SplitterLeftRight->SplitVertically(&m_PagesContainer.GetListWindow(), m_RightPane);
		rightPaneSizer->Add(m_MenuBar, 0, wxEXPAND);
		rightPaneSizer->Add(&m_PagesContainer.GetWindow(), 1, wxEXPAND|wxLEFT, KLC_HORIZONTAL_SPACING);

		// Create pages
		m_PageInfo = &m_PagesContainer.MakeWorkspace<KPackageCreatorPageInfo>(*this, m_WorkspaceDocument);
		m_PageFileData = &m_PagesContainer.MakeWorkspace<KPackageCreatorPageFileData>(*this, m_WorkspaceDocument);
		m_PageInterface = &m_PagesContainer.MakeWorkspace<KPackageCreatorPageInterface>(*this, m_WorkspaceDocument);
		m_PageRequirements = &m_PagesContainer.MakeWorkspace<KPackageCreatorPageRequirements>(*this, m_WorkspaceDocument);
		m_PageComponents = &m_PagesContainer.MakeWorkspace<KPackageCreatorPageComponents>(*this, m_WorkspaceDocument);
		return true;
	}
	bool KPackageCreatorWorkspace::OnOpenWorkspace()
	{
		if (!OpenedOnce())
		{
			m_WorkspaceDocument.NewProject();

			using namespace Application;
			GetUIOption(OName::Splitter).LoadSplitterLayout(m_SplitterLeftRight);
			GetUIOption(OName::Pages).LoadWorkspaceContainerLayout(m_PagesContainer);
		}
		return true;
	}
	bool KPackageCreatorWorkspace::OnCloseWorkspace()
	{
		return m_WorkspaceDocument.AskForSave() == KxID_OK;
	}
	void KPackageCreatorWorkspace::OnReloadWorkspace()
	{
	}

	KPackageCreatorWorkspace::KPackageCreatorWorkspace()
		:m_WorkspaceDocument(*this), m_PagesContainer(*this)
	{
		IMainWindow::GetInstance()->AddToolBarMenuItem(*this);
	}
	KPackageCreatorWorkspace::~KPackageCreatorWorkspace()
	{
		if (IsCreated())
		{
			using namespace Application;
			GetUIOption(OName::Splitter).SaveSplitterLayout(m_SplitterLeftRight);
			GetUIOption(OName::Pages).SaveWorkspaceContainerLayout(m_PagesContainer);
		}
	}

	void KPackageCreatorWorkspace::CreateMenuBar(wxSizer* sizer)
	{
		m_MenuBar = new KxAuiToolBar(m_RightPane, KxID_NONE, KxAuiToolBar::DefaultStyle|wxAUI_TB_PLAIN_BACKGROUND|wxAUI_TB_TEXT);
		m_MenuBar->SetBackgroundColour(GetBackgroundColour());
		m_MenuBar->SetBorderColor(IThemeManager::GetActive().GetColor(Theme::ColorIndex::Border, Theme::ColorFlags::Background));
		m_MenuBar->SetToolPacking(0);
		m_MenuBar->SetToolSeparation(0);
		m_MenuBar->SetMargins(0, 0, 0, 2);
		IThemeManager::GetActive().Apply(static_cast<wxWindow*>(m_MenuBar));

		m_MenuBar_Project = m_MenuBar->AddTool(KTr("PackageCreator.MenuBar.Project"), wxNullBitmap);
		m_MenuBar_Import = m_MenuBar->AddTool(KTr("PackageCreator.MenuBar.Import"), wxNullBitmap);
		m_MenuBar_Build = m_MenuBar->AddTool(KTr("PackageCreator.MenuBar.Build"), wxNullBitmap);

		m_MenuBar->UpdateUI();
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

	void KPackageCreatorWorkspace::DoLoadAllPages()
	{
		m_PageInfo->EnsureCreated();
		m_PageFileData->EnsureCreated();
		m_PageInterface->EnsureCreated();
		m_PageRequirements->EnsureCreated();
		m_PageComponents->EnsureCreated();

		m_PageInfo->OnLoadProject(m_WorkspaceDocument.GetProject()->GetInfo());
		m_PageFileData->OnLoadProject(m_WorkspaceDocument.GetProject()->GetFileData());
		m_PageInterface->OnLoadProject(m_WorkspaceDocument.GetProject()->GetInterface());
		m_PageRequirements->OnLoadProject(m_WorkspaceDocument.GetProject()->GetRequirements());
		m_PageComponents->OnLoadProject(m_WorkspaceDocument.GetProject()->GetComponents());
	}
	void KPackageCreatorWorkspace::OnNewProject(KxMenuEvent& event)
	{
		if (m_WorkspaceDocument.AskForSave() == KxID_OK)
		{
			m_WorkspaceDocument.NewProject();
			DoLoadAllPages();
		}
	}
	void KPackageCreatorWorkspace::OnOpenProject(KxMenuEvent& event)
	{
		wxWindowUpdateLocker lock(this);

		if (m_WorkspaceDocument.AskForSave() == KxID_OK)
		{
			KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_OPEN);
			dialog.AddFilter("*.kmpproj", KTr("FileFilter.ModProject"));
			dialog.AddFilter("*", KTr("FileFilter.AllFiles"));

			if (dialog.ShowModal() == KxID_OK)
			{
				m_WorkspaceDocument.OpenProject(dialog.GetResult());
				DoLoadAllPages();
			}
		}
	}
	void KPackageCreatorWorkspace::OnSaveProject(KxMenuEvent& event)
	{
		if (event.GetItem()->GetId() == KxID_SAVEAS || !m_WorkspaceDocument.HasProjectFilePath() || !wxFileName(m_WorkspaceDocument.GetProjectFilePath()).Exists(wxFILE_EXISTS_REGULAR))
		{
			KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_SAVE);
			dialog.SetDefaultExtension("kmpproj");
			dialog.SetFileName(m_WorkspaceDocument.GetProjectName());
			dialog.AddFilter("*.kmpproj", KTr("FileFilter.ModProject"));
			dialog.AddFilter("*", KTr("FileFilter.AllFiles"));

			if (dialog.ShowModal() == KxID_OK)
			{
				m_WorkspaceDocument.SaveProject(dialog.GetResult());
			}
		}
		else
		{
			m_WorkspaceDocument.SaveProject();
		}
	}
	void KPackageCreatorWorkspace::OnImportProject(KxMenuEvent& event)
	{
		wxWindowUpdateLocker lock(this);
		if (m_WorkspaceDocument.AskForSave() == KxID_OK)
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
						m_WorkspaceDocument.ImportProject(tSerailizer);
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
				dialog.SetFileName(m_WorkspaceDocument.GetProjectName());
				dialog.AddFilter("*.xml", KTr("FileFilter.XML"));
				dialog.AddFilter("*", KTr("FileFilter.AllFiles"));

				if (dialog.ShowModal() == KxID_OK)
				{
					KPackageProjectSerializerKMP serializer(false);
					m_WorkspaceDocument.ExportProject(serializer);
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
					m_WorkspaceDocument.ExportProject(serializer);

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
		m_WorkspaceDocument.BuildProject();
	}
	void KPackageCreatorWorkspace::OnBuildProjectPreview(KxMenuEvent& event)
	{
		m_WorkspaceDocument.BuildProject(true);
	}

	bool KPackageCreatorWorkspace::QueryInterface(const KxIID& iid, void*& ptr) noexcept
	{
		return UseAnyOf<IWorkspaceDocument, IWorkspaceContainer>(iid, ptr, m_WorkspaceDocument, m_PagesContainer) || QuerySelf(iid, ptr, *this);
	}

	wxString KPackageCreatorWorkspace::GetID() const
	{
		return "PackageDesigner::Workspace";
	}
	wxString KPackageCreatorWorkspace::GetName() const
	{
		return KTr("PackageManager.CreatorName");
	}
	IWorkspaceContainer* KPackageCreatorWorkspace::GetPreferredContainer() const
	{
		return &IMainWindow::GetInstance()->GetWorkspaceContainer();
	}

	KPackageCreatorPageBase* KPackageCreatorWorkspace::GetCurrentPage() const
	{
		return static_cast<KPackageCreatorPageBase*>(m_PagesContainer.GetCurrentWorkspace());
	}
	KPackageProject& KPackageCreatorWorkspace::ImportProjectFromPackage(const wxString& path)
	{
		EnsureCreated();

		m_WorkspaceDocument.ImportProjectFromPackage(path);
		return *m_WorkspaceDocument.GetProject();
	}
	KPackageProject& KPackageCreatorWorkspace::CreateProjectFromModEntry(const Kortex::IGameMod& modEntry)
	{
		EnsureCreated();

		m_WorkspaceDocument.CreateProjectFromModEntry(modEntry);
		return *m_WorkspaceDocument.GetProject();
	}
}
