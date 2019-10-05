#include "stdafx.h"
#include "KPackageCreatorPageInterface.h"
#include "KPackageCreatorPageBase.h"
#include "PageInterface/KPCIImagesListModel.h"
#include "KPackageCreatorWorkspace.h"
#include "KPackageCreatorController.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxComboBox.h>

namespace Kortex::PackageDesigner
{
	KPackageProjectInterface& KPackageCreatorPageInterface::GetProjectInterface() const
	{
		return GetProject()->GetInterface();
	}
	void KPackageCreatorPageInterface::OnLoadProject(KPackageProjectInterface& projectInterface)
	{
		wxWindowUpdateLocker lock(this);

		m_ImageListModel->SetProject(projectInterface.GetProject());
		m_ImageListModel->SetDataVector(&projectInterface.GetImages());
	}
	void KPackageCreatorPageInterface::CreateImageListControls()
	{
		// Main caption
		KxLabel* label = CreateCaptionLabel(this, KTr("PackageCreator.PageInterface.ImageList"));
		m_MainSizer->Add(label, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

		// Sizer
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		m_MainSizer->Add(sizer, 1, wxEXPAND|wxLEFT, ms_LeftMargin);

		// List
		m_ImageListModel = new KPCIImagesListModel();
		m_ImageListModel->Create(m_Controller, this, sizer);
	}
	
	bool KPackageCreatorPageInterface::OnCreateWorkspace()
	{
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
		SetSizer(m_MainSizer);

		CreateImageListControls();

		//KProgramOptionSerializer::LoadDataViewLayout(m_ImageListModel->GetView(), m_ListOptions);
		return true;
	}
	bool KPackageCreatorPageInterface::OnOpenWorkspace()
	{
		return true;
	}
	bool KPackageCreatorPageInterface::OnCloseWorkspace()
	{
		return true;
	}
	
	KPackageCreatorPageInterface::KPackageCreatorPageInterface(KPackageCreatorWorkspace& mainWorkspace, KPackageCreatorController& controller)
		:KPackageCreatorPageBase(mainWorkspace, controller)
		//m_MainOptions(this, "MainUI"), m_ListOptions(this, "ListView")
	{
	}
	KPackageCreatorPageInterface::~KPackageCreatorPageInterface()
	{
		if (IsCreated())
		{
			//KProgramOptionSerializer::SaveDataViewLayout(m_ImageListModel->GetView(), m_ListOptions);
			m_ImageListModel->SetDataVector();
		}
	}

	wxString KPackageCreatorPageInterface::GetID() const
	{
		return "KPackageCreator.PageInterface";
	}
	wxString KPackageCreatorPageInterface::GetPageName() const
	{
		return KTr("PackageCreator.PageInterface.Name");
	}
}
