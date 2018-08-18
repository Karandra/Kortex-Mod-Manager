#include "stdafx.h"
#include "KPackageCreatorPageComponents.h"
#include "KPackageCreatorPageBase.h"
#include "KPackageCreatorWorkspace.h"
#include "KPackageCreatorController.h"
#include "PackageCreator/KPackageCreatorPageFileData.h"
#include "PageComponents/KPCCFileDataSelectorModel.h"
#include "PageComponents/KPCCConditionalStepsModel.h"
#include "PageComponents/KPCComponentsModel.h"
#include "UI/KImageViewerDialog.h"
#include "KThemeManager.h"
#include "KAux.h"
#include <KxFramework/KxString.h>
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxComboBox.h>
#include <KxFramework/KxDataViewComboBox.h>

wxString KPackageCreatorPageComponents::FormatArrayToText(const KxStringVector& array)
{
	return array.empty() ? KAux::MakeNoneLabel() : KxString::Join(array, ", ");
}
wxString KPackageCreatorPageComponents::FormatArrayToText(const KPPCFlagEntryArray& array, bool isRequired)
{
	if (!array.empty())
	{
		wxString out;
		for (size_t i = 0; i < array.size(); i++)
		{
			const KPPCFlagEntry& flag = array[i];
			out.Append(wxString::Format("%s %s \"%s\"", flag.GetName(), (isRequired ? "==" : "="), flag.GetValue()));

			if (i + 1 != array.size())
			{
				if (isRequired)
				{
					out.Append(' ' + KPackageProjectRequirements::OperatorToSymbolicName(flag.GetOperator()) + ' ');
				}
				else
				{
					out.Append(", ");
				}
			}
		}
		return out;
	}
	return KAux::MakeNoneLabel();
}

KPackageCreatorPageComponents::KPackageCreatorPageComponents(KPackageCreatorWorkspace* mainWorkspace, KPackageCreatorController* controller)
	:KPackageCreatorPageBase(mainWorkspace, controller),
	m_MainOptions(this, "MainUI"), m_ComponnetsOptions(this, "ComponentsView")
{
}
KPackageCreatorPageComponents::~KPackageCreatorPageComponents()
{
	if (IsWorkspaceCreated())
	{
		KProgramOptionSerializer::SaveDataViewLayout(m_ComponentsModel->GetView(), m_ComponnetsOptions);
	}
}
bool KPackageCreatorPageComponents::OnCreateWorkspace()
{
	CreateComponentsView();
	CreateMiscControls();

	KProgramOptionSerializer::LoadDataViewLayout(m_ComponentsModel->GetView(), m_ComponnetsOptions);
	return true;
}
KPackageProjectComponents& KPackageCreatorPageComponents::GetProjectComponents() const
{
	return GetProject()->GetComponents();
}

void KPackageCreatorPageComponents::CreateComponentsView()
{
	// Main caption
	KxLabel* label = CreateCaptionLabel(this, T("PackageCreator.PageComponents.Name"));
	m_MainSizer->Add(label, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	m_MainSizer->Add(sizer, 1, wxEXPAND|wxLEFT, ms_LeftMargin);

	m_ComponentsModel = new KPCComponentsModel(m_Controller);
	m_ComponentsModel->Create(this, sizer);
}
void KPackageCreatorPageComponents::CreateMiscControls()
{
	wxBoxSizer* stepsSizer = new wxBoxSizer(wxHORIZONTAL);
	m_MainSizer->Add(stepsSizer, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);

	// Main caption
	wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
	stepsSizer->Add(leftSizer, 1, wxEXPAND);

	KxLabel* label = CreateCaptionLabel(this, T("PackageCreator.PageComponents.Advanced"));
	leftSizer->Add(label, 0, wxEXPAND|wxBOTTOM, KLC_VERTICAL_SPACING);

	// Entry image
	m_EntryImage = new KxImageView(this, KxID_NONE, wxBORDER_THEME);
	m_EntryImage->SetMinSize(wxSize(192, 108));
	m_EntryImage->SetScaleMode(KxIV_SCALE_ASPECT_FIT);
	m_EntryImage->Bind(wxEVT_LEFT_DCLICK, [this](wxMouseEvent& event)
	{
		event.Skip();
		if (const KPPIImageEntry* imageEntry = static_cast<const KPPIImageEntry*>(m_EntryImage->GetClientData()))
		{
			KImageViewerDialog dialog(GetMainWindow());

			KImageViewerEvent event;
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
	wxBoxSizer* sizer = AddControlsRow2(this, leftTableSizer, T("PackageCreator.PageComponents.RequiredFiles"), new wxBoxSizer(wxVERTICAL));

	m_RequiredFilesModel = new KPCCFileDataSelectorModelCB();
	m_RequiredFilesModel->Create(m_Controller, this, sizer);
	m_RequiredFilesModel->SetDataVector();

	KxButton* conditionalSteps = AddControlsRow(leftTableSizer, T("PackageCreator.PageComponents.ConditionalInstall"), new KxButton(this, KxID_NONE, T(KxID_EDIT)), 0);
	conditionalSteps->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
	{
		KPCCConditionalStepsModelDialog dialog(GetMainWindow(), T("PackageCreator.PageComponents.ConditionalInstall"), m_Controller, true);
		dialog.SetDataVector(GetProjectComponents().GetConditionalSteps());
		dialog.ShowModal();
	});
}

bool KPackageCreatorPageComponents::OnOpenWorkspace()
{
	KPackageProjectComponents& projectComponents = GetProjectComponents();
	m_RequiredFilesModel->SetDataVector(projectComponents.GetRequiredFileData(), &projectComponents.GetProject().GetFileData());
	return true;
}
bool KPackageCreatorPageComponents::OnCloseWorkspace()
{
	return true;
}
void KPackageCreatorPageComponents::OnLoadProject(KPackageProjectComponents& projectComponents)
{
	wxWindowUpdateLocker lock(this);
	m_ComponentsModel->SetProject(projectComponents.GetProject());
}

wxString KPackageCreatorPageComponents::GetID() const
{
	return "KPackageCreator.PageComponents";
}
wxString KPackageCreatorPageComponents::GetPageName() const
{
	return T("PackageCreator.PageComponents.Name");
}
