#include "stdafx.h"
#include "KPackageCreatorPageFileData.h"
#include "KPackageCreatorPageBase.h"
#include "KPackageCreatorWorkspace.h"
#include "KPackageCreatorController.h"
#include "PageFileData/KPCFileDataMainListModel.h"
#include "PageFileData/KPCFileDataFolderContentModel.h"
#include "PackageProject/KPackageProject.h"
#include "PackageProject/KPackageProjectFileData.h"
#include "KThemeManager.h"
#include <KxFramework/KxLabel.h>

KPackageCreatorPageFileData::KPackageCreatorPageFileData(KPackageCreatorWorkspace* mainWorkspace, KPackageCreatorController* controller)
	:KPackageCreatorPageBase(mainWorkspace, controller),
	m_MainOptions(this, "MainUI"), m_MainListOptions(this, "FolderListView"), m_ContentListModelOptions(this, "FolderContentView")
{
}
KPackageCreatorPageFileData::~KPackageCreatorPageFileData()
{
	if (IsWorkspaceCreated())
	{
		KProgramOptionSerializer::SaveDataViewLayout(m_MainListModel->GetView(), m_MainListOptions);
		KProgramOptionSerializer::SaveDataViewLayout(m_ContentListModel->GetView(), m_ContentListModelOptions);
		KProgramOptionSerializer::SaveSplitterLayout(m_Pane, m_MainOptions);

		m_MainListModel->SetDataVector();
		m_ContentListModel->SetDataVector();
	}
}
bool KPackageCreatorPageFileData::OnCreateWorkspace()
{
	m_Pane = new KxSplitterWindow(this, KxID_NONE);
	m_Pane->SetName("FolderListViewSize");
	m_Pane->SetMinimumPaneSize(150);
	m_MainSizer->Add(m_Pane, 1, wxEXPAND);
	m_Pane->SetSashColor(KThemeManager::Get().GetColor(KTMC_WINDOW_BG));

	CreateMainListControls();
	CreateFolderContentControls();
	m_Pane->SplitHorizontally(m_MainListPane, m_FolderContentPane, 0);

	KProgramOptionSerializer::LoadDataViewLayout(m_MainListModel->GetView(), m_MainListOptions);
	KProgramOptionSerializer::LoadDataViewLayout(m_ContentListModel->GetView(), m_ContentListModelOptions);
	KProgramOptionSerializer::LoadSplitterLayout(m_Pane, m_MainOptions);
	return true;
}
KPackageProjectFileData& KPackageCreatorPageFileData::GetProjectFileData() const
{
	return GetProject()->GetFileData();
}

void KPackageCreatorPageFileData::CreateMainListControls()
{
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	m_MainListPane = new KxPanel(m_Pane, KxID_NONE);
	m_MainListPane->SetSizer(mainSizer);
	KThemeManager::Get().ProcessWindow(m_MainListPane);

	// Main caption
	KxLabel* label = CreateCaptionLabel(m_MainListPane, T("PackageCreator.PageFileData.MainList"));
	mainSizer->Add(label, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

	// Sizer
	wxBoxSizer* foldersSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(foldersSizer, 1, wxEXPAND|wxLEFT, ms_LeftMargin);

	// List
	m_MainListModel = new KPCFileDataMainListModel();
	m_MainListModel->Create(m_Controller, m_MainListPane, foldersSizer);
}
void KPackageCreatorPageFileData::CreateFolderContentControls()
{
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	m_FolderContentPane = new KxPanel(m_Pane, KxID_NONE);
	m_FolderContentPane->SetSizer(mainSizer);
	KThemeManager::Get().ProcessWindow(m_FolderContentPane);

	// Main caption
	KxLabel* label = CreateCaptionLabel(m_FolderContentPane, T("PackageCreator.PageFileData.FolderContent"));
	mainSizer->Add(label, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

	wxBoxSizer* filesSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(filesSizer, 1, wxEXPAND|wxLEFT, ms_LeftMargin);

	m_ContentListModel = new KPCFileDataFolderContentModel();
	m_ContentListModel->Create(m_Controller, m_FolderContentPane, filesSizer);
	m_MainListModel->SetContentModel(m_ContentListModel);
}

bool KPackageCreatorPageFileData::OnOpenWorkspace()
{
	return true;
}
bool KPackageCreatorPageFileData::OnCloseWorkspace()
{
	return true;
}
void KPackageCreatorPageFileData::OnLoadProject(KPackageProjectFileData& projectFileData)
{
	wxWindowUpdateLocker lock(this);

	m_MainListModel->SetProject(projectFileData.GetProject());
	m_MainListModel->SetDataVector(&projectFileData.GetData());

	m_ContentListModel->SetProject(projectFileData.GetProject());
	m_ContentListModel->SetDataVector();
}

wxString KPackageCreatorPageFileData::GetID() const
{
	return "KPackageCreator.FileData";
}
wxString KPackageCreatorPageFileData::GetPageName() const
{
	return T("PackageCreator.PageFileData.Name");
}
