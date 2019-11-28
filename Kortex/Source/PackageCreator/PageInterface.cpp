#include "stdafx.h"
#include "PageInterface.h"
#include "PageBase.h"
#include "PageInterface/ImageListModel.h"
#include "Workspace.h"
#include "WorkspaceDocument.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxComboBox.h>

namespace Kortex::PackageDesigner
{
	PackageProject::InterfaceSection& PageInterface::GetProjectInterface() const
	{
		return GetProject()->GetInterface();
	}
	void PageInterface::OnLoadProject(PackageProject::InterfaceSection& projectInterface)
	{
		wxWindowUpdateLocker lock(this);

		m_ImageListModel->SetProject(projectInterface.GetProject());
		m_ImageListModel->SetDataVector(&projectInterface.GetImages());
	}
	void PageInterface::CreateImageListControls()
	{
		// Main caption
		KxLabel* label = CreateCaptionLabel(this, KTr("PackageCreator.PageInterface.ImageList"));
		m_MainSizer->Add(label, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

		// Sizer
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		m_MainSizer->Add(sizer, 1, wxEXPAND|wxLEFT, ms_LeftMargin);

		// List
		m_ImageListModel = new PageInterfaceNS::ImageListModel();
		m_ImageListModel->Create(m_Controller, this, sizer);
	}
	
	bool PageInterface::OnCreateWorkspace()
	{
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
		SetSizer(m_MainSizer);

		CreateImageListControls();

		//KProgramOptionSerializer::LoadDataViewLayout(m_ImageListModel->GetView(), m_ListOptions);
		return true;
	}
	bool PageInterface::OnOpenWorkspace()
	{
		return true;
	}
	bool PageInterface::OnCloseWorkspace()
	{
		return true;
	}
	
	PageInterface::PageInterface(Workspace& mainWorkspace, WorkspaceDocument& controller)
		:PageBase(mainWorkspace, controller)
		//m_MainOptions(this, "MainUI"), m_ListOptions(this, "ListView")
	{
	}
	PageInterface::~PageInterface()
	{
		if (IsCreated())
		{
			//KProgramOptionSerializer::SaveDataViewLayout(m_ImageListModel->GetView(), m_ListOptions);
			m_ImageListModel->SetDataVector();
		}
	}

	wxString PageInterface::GetID() const
	{
		return "KPackageCreator.PageInterface";
	}
	wxString PageInterface::GetPageName() const
	{
		return KTr("PackageCreator.PageInterface.Name");
	}
}
