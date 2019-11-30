#include "stdafx.h"
#include "PageFileData.h"
#include "PageBase.h"
#include "Workspace.h"
#include "WorkspaceDocument.h"
#include "PageFileData/MainListModel.h"
#include "PageFileData/ContentModel.h"
#include "PackageProject/ModPackageProject.h"
#include "PackageProject/FileDataSection.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxLabel.h>

namespace Kortex::PackageDesigner
{
	void PageFileData::OnLoadProject(PackageProject::FileDataSection& projectFileData)
	{
		wxWindowUpdateLocker lock(this);

		m_MainListModel->SetProject(projectFileData.GetProject());
		m_MainListModel->SetDataVector(&projectFileData.GetItems());

		m_ContentListModel->SetProject(projectFileData.GetProject());
		m_ContentListModel->SetDataVector();
	}
	PackageProject::FileDataSection& PageFileData::GetProjectFileData() const
	{
		return GetProject()->GetFileData();
	}

	void PageFileData::CreateMainListControls()
	{
		wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
		m_MainListPane = new KxPanel(m_Pane, KxID_NONE);
		m_MainListPane->SetSizer(mainSizer);
		IThemeManager::GetActive().Apply(m_MainListPane);

		// Main caption
		KxLabel* label = CreateCaptionLabel(m_MainListPane, KTr("PackageCreator.PageFileData.MainList"));
		mainSizer->Add(label, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

		// Sizer
		wxBoxSizer* foldersSizer = new wxBoxSizer(wxVERTICAL);
		mainSizer->Add(foldersSizer, 1, wxEXPAND|wxLEFT, ms_LeftMargin);

		// List
		m_MainListModel = new PageFileDataNS::MainListModel();
		m_MainListModel->Create(m_Controller, m_MainListPane, foldersSizer);
	}
	void PageFileData::CreateFolderContentControls()
	{
		wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
		m_FolderContentPane = new KxPanel(m_Pane, KxID_NONE);
		m_FolderContentPane->SetSizer(mainSizer);
		IThemeManager::GetActive().Apply(m_FolderContentPane);

		// Main caption
		KxLabel* label = CreateCaptionLabel(m_FolderContentPane, KTr("PackageCreator.PageFileData.FolderContent"));
		mainSizer->Add(label, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

		wxBoxSizer* filesSizer = new wxBoxSizer(wxVERTICAL);
		mainSizer->Add(filesSizer, 1, wxEXPAND|wxLEFT, ms_LeftMargin);

		m_ContentListModel = new PageFileDataNS::ContentModel();
		m_ContentListModel->Create(m_Controller, m_FolderContentPane, filesSizer);
		m_MainListModel->SetContentModel(m_ContentListModel);
	}

	bool PageFileData::OnCreateWorkspace()
	{
		m_Pane = new KxSplitterWindow(this, KxID_NONE);
		m_Pane->SetName("FolderListViewSize");
		m_Pane->SetMinimumPaneSize(150);
		m_Pane->SetSashColor(IThemeManager::GetActive().GetColor(Theme::ColorIndex::Window, Theme::ColorFlags::Background));

		CreateMainListControls();
		CreateFolderContentControls();
		m_Pane->SplitHorizontally(m_MainListPane, m_FolderContentPane, 0);

		//KProgramOptionSerializer::LoadDataViewLayout(m_MainListModel->GetView(), m_MainListOptions);
		//KProgramOptionSerializer::LoadDataViewLayout(m_ContentListModel->GetView(), m_ContentListModelOptions);
		//KProgramOptionSerializer::LoadSplitterLayout(m_Pane, m_MainOptions);
		return true;
	}
	bool PageFileData::OnOpenWorkspace()
	{
		return true;
	}
	bool PageFileData::OnCloseWorkspace()
	{
		return true;
	}
	
	PageFileData::PageFileData(Workspace& mainWorkspace, WorkspaceDocument& controller)
		:PageBase(mainWorkspace, controller)
		//m_MainOptions(this, "MainUI"), m_MainListOptions(this, "FolderListView"), m_ContentListModelOptions(this, "FolderContentView")
	{
	}
	PageFileData::~PageFileData()
	{
		if (IsCreated())
		{
			//KProgramOptionSerializer::SaveDataViewLayout(m_MainListModel->GetView(), m_MainListOptions);
			//KProgramOptionSerializer::SaveDataViewLayout(m_ContentListModel->GetView(), m_ContentListModelOptions);
			//KProgramOptionSerializer::SaveSplitterLayout(m_Pane, m_MainOptions);

			m_MainListModel->SetDataVector();
			m_ContentListModel->SetDataVector();
		}
	}
	
	wxString PageFileData::GetID() const
	{
		return "KPackageCreator.FileData";
	}
	wxString PageFileData::GetPageName() const
	{
		return KTr("PackageCreator.PageFileData.Name");
	}
}
