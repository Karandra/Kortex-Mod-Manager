#include "stdafx.h"
#include "PageComponents.h"
#include "PageBase.h"
#include "Workspace.h"
#include "WorkspaceDocument.h"
#include "PackageCreator/PageFileData.h"
#include "PageComponents/FileDataSelectorModel.h"
#include "PageComponents/ConditionalStepsModel.h"
#include "PageComponents/ComponentsModel.h"
#include "UI/ImageViewerDialog.h"
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxComboBox.h>
#include <KxFramework/KxDataViewComboBox.h>

namespace Kortex::PackageDesigner
{
	wxString PageComponents::FormatArrayToText(const KxStringVector& array)
	{
		return array.empty() ? wxEmptyString : KxString::Join(array, wxS(", "));
	}
	wxString PageComponents::ConditionToString(const PackageProject::Condition& condition, bool isRequired)
	{
		wxString out;

		const PackageProject::FlagItem::Vector& flags = condition.GetFlags();
		for (size_t i = 0; i < flags.size(); i++)
		{
			const PackageProject::FlagItem& flag = flags[i];
			out.Append(KxString::Format(wxS("%1 %2 \"%3\""), flag.GetName(), (isRequired ? wxS("==") : wxS("=")), flag.GetValue()));

			if (i + 1 != flags.size())
			{
				if (isRequired)
				{
					out += wxS(" ");
					out += ModPackageProject::OperatorToSymbolicName(condition.GetOperator());
					out += wxS(" ");
				}
				else
				{
					out += wxS(", ");
				}
			}
		}
		return out;
	}
	wxString PageComponents::ConditionGroupToString(const PackageProject::ConditionGroup& conditionGroup)
	{
		wxString out;
		if (conditionGroup.HasConditions())
		{
			const PackageProject::Condition::Vector& conditions = conditionGroup.GetConditions();
			for (size_t i = 0; i < conditions.size(); i++)
			{
				out.Append(wxS('(') + ConditionToString(conditions[i], true) + wxS(')'));
				if (i + 1 != conditions.size())
				{
					out += wxS(" ");
					out += ModPackageProject::OperatorToSymbolicName(conditionGroup.GetOperator());
					out += wxS(" ");
				}
			}
		}
		return out;
	}

	PackageProject::ComponentsSection& PageComponents::GetProjectComponents() const
	{
		return GetProject()->GetComponents();
	}
	void PageComponents::OnLoadProject(PackageProject::ComponentsSection& projectComponents)
	{
		wxWindowUpdateLocker lock(this);
		m_ComponentsModel->SetProject(projectComponents.GetProject());
	}

	void PageComponents::CreateComponentsView()
	{
		// Main caption
		KxLabel* label = CreateCaptionLabel(this, KTr("PackageCreator.PageComponents.Name"));
		m_MainSizer->Add(label, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

		wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		m_MainSizer->Add(sizer, 1, wxEXPAND|wxLEFT, ms_LeftMargin);

		m_ComponentsModel = new PageComponentsNS::ComponentsModel(m_Controller);
		m_ComponentsModel->Create(this, sizer);
	}
	void PageComponents::CreateMiscControls()
	{
		wxBoxSizer* stepsSizer = new wxBoxSizer(wxHORIZONTAL);
		m_MainSizer->Add(stepsSizer, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);

		// Main caption
		wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
		stepsSizer->Add(leftSizer, 1, wxEXPAND);

		KxLabel* label = CreateCaptionLabel(this, KTr("PackageCreator.PageComponents.Advanced"));
		leftSizer->Add(label, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

		// Entry image
		m_EntryImage = new KxImageView(this, KxID_NONE, wxBORDER_THEME);
		m_EntryImage->SetMinSize(wxSize(192, 108));
		m_EntryImage->SetScaleMode(KxIV_SCALE_ASPECT_FIT);
		m_EntryImage->Bind(wxEVT_LEFT_DCLICK, [this](wxMouseEvent& event)
		{
			event.Skip();
			if (const PackageProject::ImageItem* imageEntry = static_cast<const PackageProject::ImageItem*>(m_EntryImage->GetClientData()))
			{
				UI::ImageViewerDialog dialog(this);

				UI::ImageViewerEvent event;
				event.SetFilePath(imageEntry->GetPath());
				dialog.Navigate(event);
				dialog.ShowModal();
			}
		});
		stepsSizer->Add(m_EntryImage, 0, wxEXPAND|wxLEFT, KLC_HORIZONTAL_SPACING)->SetRatio(1.77f);
		m_ComponentsModel->SetEntryImageView(m_EntryImage);

		// Sizer
		wxFlexGridSizer* leftTableSizer = new wxFlexGridSizer(2, KLC_VERTICAL_SPACING, KLC_HORIZONTAL_SPACING);
		leftTableSizer->AddGrowableCol(1, 1);
		leftSizer->Add(leftTableSizer, 1, wxEXPAND|wxLEFT, ms_LeftMargin);

		// Required files
		wxBoxSizer* sizer = AddControlsRow2(this, leftTableSizer, KTr("PackageCreator.PageComponents.RequiredFiles"), new wxBoxSizer(wxVERTICAL));

		m_RequiredFilesModel = new PageComponentsNS::FileDataSelectorComboBox();
		m_RequiredFilesModel->Create(m_Controller, this, sizer);
		m_RequiredFilesModel->SetDataVector();

		KxButton* conditionalSteps = AddControlsRow(leftTableSizer, KTr("PackageCreator.PageComponents.ConditionalInstall"), new KxButton(this, KxID_NONE, KTr(KxID_EDIT)), 0);
		conditionalSteps->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
		{
			PageComponentsNS::ConditionalStepsDialog dialog(this, KTr("PackageCreator.PageComponents.ConditionalInstall"), m_Controller, true);
			dialog.SetDataVector(GetProjectComponents().GetConditionalSteps());
			dialog.ShowModal();
		});
	}

	bool PageComponents::OnCreateWorkspace()
	{
		m_MainSizer = new wxBoxSizer(wxVERTICAL);
		SetSizer(m_MainSizer);

		CreateComponentsView();
		CreateMiscControls();

		//KProgramOptionSerializer::LoadDataViewLayout(m_ComponentsModel->GetView(), m_ComponentsOptions);
		return true;
	}
	bool PageComponents::OnOpenWorkspace()
	{
		PackageProject::ComponentsSection& projectComponents = GetProjectComponents();
		m_RequiredFilesModel->SetDataVector(projectComponents.GetRequiredFileData(), &projectComponents.GetProject().GetFileData());
		return true;
	}
	bool PageComponents::OnCloseWorkspace()
	{
		return true;
	}
	
	PageComponents::PageComponents(Workspace& mainWorkspace, WorkspaceDocument& controller)
		:PageBase(mainWorkspace, controller)
		//m_MainOptions(this, "MainUI"), m_ComponentsOptions(this, "ComponentsView")
	{
	}
	PageComponents::~PageComponents()
	{
		if (IsCreated())
		{
			//KProgramOptionSerializer::SaveDataViewLayout(m_ComponentsModel->GetView(), m_ComponentsOptions);
		}
	}

	wxString PageComponents::GetID() const
	{
		return "KPackageCreator.PageComponents";
	}
	wxString PageComponents::GetPageName() const
	{
		return KTr("PackageCreator.PageComponents.Name");
	}
}
