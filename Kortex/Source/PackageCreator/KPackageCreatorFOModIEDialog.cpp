#include "stdafx.h"
#include "KPackageCreatorFOModIEDialog.h"
#include "PackageCreator/KPackageCreatorPageBase.h"
#include <Kortex/Application.hpp>
#include "KAux.h"
#include <KxFramework/KxFileBrowseDialog.h>

void KPackageCreatorFOModIEDialog::OnText(wxCommandEvent& event)
{
	m_OKButton->Enable(!m_InfoInput->IsEmpty() || !m_ModuleConfigInput->IsEmpty());
}
void KPackageCreatorFOModIEDialog::OnBrowseFile(wxCommandEvent& event)
{
	KxFBD_Mode mode = event.GetId() == ProjectFolder ? KxFBD_OPEN_FOLDER : KxFBD_OPEN;
	if (mode == KxFBD_OPEN && m_IsExport)
	{
		mode = KxFBD_SAVE;
	}

	KxFileBrowseDialog dialog(this, KxID_NONE, mode);
	if (event.GetId() != ProjectFolder)
	{
		wxStringClientData* data = static_cast<wxStringClientData*>(static_cast<wxEvtHandler*>(event.GetEventObject())->GetClientObject());
		if (m_IsExport && data)
		{
			dialog.SetFileName(data->GetData());
		}

		dialog.SetDefaultExtension("xml");
		dialog.AddFilter("*.xml", KTr("FileFilter.XML"));
		dialog.AddFilter("*", KTr("FileFilter.AllFiles"));
	}

	wxWindowID id = dialog.ShowModal();
	KxTextBox* input = nullptr;
	switch (event.GetId())
	{
		case InfoXML:
		{
			input = m_InfoInput;
			break;
		}
		case ModuleConfigXML:
		{
			input = m_ModuleConfigInput;
			break;
		}
		case ProjectFolder:
		{
			input = m_ProjectFolderInput;
			break;
		}
	};

	if (input)
	{
		input->SetValueEvent(id == KxID_OK ? dialog.GetResult() : wxEmptyString);
	}
}
void KPackageCreatorFOModIEDialog::OnOK(wxNotifyEvent& event)
{
	if (event.GetId() == KxID_OK)
	{
		m_InfoFile = m_InfoInput->GetValue();
		m_ModuleConfigFile = m_ModuleConfigInput->GetValue();
		m_ProjectFolder = m_ProjectFolderInput->GetValue();
	}
}

KPackageCreatorFOModIEDialog::KPackageCreatorFOModIEDialog(wxWindow* parent, bool isExport)
	:m_IsExport(isExport)
{
	if (KxStdDialog::Create(parent, KxID_NONE, isExport ? KTr("PackageCreator.ExportFOMod") : KTr("PackageCreator.ImportFOMod"), wxDefaultPosition, wxDefaultSize, KxBTN_OK|KxBTN_CANCEL))
	{
		SetMainIcon(KxICON_NONE);
		SetWindowResizeSide(wxHORIZONTAL);

		wxFlexGridSizer* mainSizer = new wxFlexGridSizer(3, KLC_VERTICAL_SPACING, KLC_HORIZONTAL_SPACING);
		mainSizer->AddGrowableCol(1, 1);
		m_ViewPane = new KxPanel(GetContentWindow(), KxID_NONE);
		m_ViewPane->SetSizer(mainSizer);
		PostCreate();

		// Info.xml
		m_InfoInput = KPackageCreatorPageBase::AddControlsRow(mainSizer, "Info.xml", new KxTextBox(m_ViewPane, KxID_NONE));
		m_InfoInput->Bind(wxEVT_TEXT, &KPackageCreatorFOModIEDialog::OnText, this);

		KxButton* infoButton = new KxButton(m_ViewPane, InfoXML, KTr(KxID_SELECT_FILE));
		infoButton->Bind(wxEVT_BUTTON, &KPackageCreatorFOModIEDialog::OnBrowseFile, this);
		infoButton->SetClientObject(new wxStringClientData("Info"));
		mainSizer->Add(infoButton, 0, wxEXPAND);

		// ModuleConfig.xml
		m_ModuleConfigInput = KPackageCreatorPageBase::AddControlsRow(mainSizer, "ModuleConfig.xml", new KxTextBox(m_ViewPane, KxID_NONE));
		m_ModuleConfigInput->Bind(wxEVT_TEXT, &KPackageCreatorFOModIEDialog::OnText, this);

		KxButton* moduleConfigButton = new KxButton(m_ViewPane, ModuleConfigXML, KTr(KxID_SELECT_FILE));
		moduleConfigButton->Bind(wxEVT_BUTTON, &KPackageCreatorFOModIEDialog::OnBrowseFile, this);
		moduleConfigButton->SetClientObject(new wxStringClientData("ModuleConfig"));
		mainSizer->Add(moduleConfigButton, 0, wxEXPAND);
		
		// Project folder path
		m_ProjectFolderInput = KPackageCreatorPageBase::AddControlsRow(mainSizer, KTr("PackageCreator.ProjectFolder"), new KxTextBox(m_ViewPane, KxID_NONE));
		m_ProjectFolderInput->Bind(wxEVT_TEXT, &KPackageCreatorFOModIEDialog::OnText, this);

		KxButton* projectFolderButton = new KxButton(m_ViewPane, ProjectFolder, KTr(KxID_SELECT_FOLDER));
		projectFolderButton->Bind(wxEVT_BUTTON, &KPackageCreatorFOModIEDialog::OnBrowseFile, this);
		mainSizer->Add(projectFolderButton, 0, wxEXPAND);

		// OK button
		m_OKButton = GetButton(KxID_OK).GetControl();
		m_OKButton->Enable(false);

		Bind(KxEVT_STDDIALOG_BUTTON, &KPackageCreatorFOModIEDialog::OnOK, this);
		AdjustWindow(wxDefaultPosition);
		SetInitialSize(wxSize(600, wxDefaultCoord));
		CenterOnScreen();
	}
}
KPackageCreatorFOModIEDialog::~KPackageCreatorFOModIEDialog()
{
}
