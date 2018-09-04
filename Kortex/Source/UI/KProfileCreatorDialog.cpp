#include "stdafx.h"
#include "KProfileCreatorDialog.h"
#include "ProgramManager/KProgramManager.h"
#include "Events/KLogEvent.h"
#include "Profile/KProfile.h"
#include "KApp.h"
#include <KxFramework/KxTaskDialog.h>

bool KProfileCreatorDialog::Create(wxWindow* parent,
								   wxWindowID id,
								   const wxString& caption,
								   const wxPoint& pos,
								   const wxSize& size,
								   int buttons,
								   long style
)
{
	if (KxComboBoxDialog::Create(parent, id, caption, pos, size, buttons, style))
	{
		SetMainIcon(KxICON_NONE);
		SetWindowResizeSide((wxOrientation)0);
		wxWindow* canvas = GetContentWindow();
		wxSizer* mainSizer = GetContentWindowSizer();
		m_ProfilesList = static_cast<KxComboBox*>(GetDialogMainCtrl());

		// Name
		wxBoxSizer* nameSizer = new wxBoxSizer(wxHORIZONTAL);
		mainSizer->Add(nameSizer, 0, wxEXPAND);

		KxLabel* nameLabel = new KxLabel(canvas, KxID_NONE, T("ProfileCreatorDialog.NameLabel"));
		nameSizer->Add(nameLabel, 1);

		m_NameInput = new KxTextBox(canvas, KxID_NONE);
		nameSizer->Add(m_NameInput, 1);

		// Copy from
		wxBoxSizer* copySizer = new wxBoxSizer(wxHORIZONTAL);
		mainSizer->Add(copySizer, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING_SMALL);

		KxLabel* copyFromLabel = new KxLabel(canvas, KxID_NONE, T("ProfileCreatorDialog.CopyFromLabel"));
		copySizer->Add(copyFromLabel, 1, wxTOP, KLC_VERTICAL_SPACING_SMALL);

		wxBoxSizer* copyOptionsSizer = new wxBoxSizer(wxVERTICAL);
		copySizer->Add(copyOptionsSizer, 1, wxEXPAND);

		mainSizer->Detach(m_ProfilesList);
		copyOptionsSizer->Add(m_ProfilesList, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING_SMALL);
		m_ProfilesList->AddItem(V("<$T(ID_NONE)>"));
		for (const wxString& name: m_Template->GetConfigsList())
		{
			m_ProfilesList->AddItem(name);
		}
		m_ProfilesList->SetSelection(0);

		// Copy profile config
		m_CopyProfileConfigCheckBox = new KxCheckBox(canvas, KxID_NONE, T("ProfileCreatorDialog.CopyProfileConfigLabel"));
		copyOptionsSizer->Add(m_CopyProfileConfigCheckBox, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING_SMALL);

		// Copy ProgramManager programs
		m_CopyRunManagerProgramsCheckBox = new KxCheckBox(canvas, KxID_NONE, T("ProfileCreatorDialog.CopyRunManagerPrograms"));
		copyOptionsSizer->Add(m_CopyRunManagerProgramsCheckBox, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING_SMALL);

		// Copy game config
		m_CopyGameConfigCheckBox = new KxCheckBox(canvas, KxID_NONE, T("ProfileCreatorDialog.CopyGameConfigLabel"));
		copyOptionsSizer->Add(m_CopyGameConfigCheckBox, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING_SMALL);

		// Copy mods
		m_CopyModsCheckBox = new KxCheckBox(canvas, KxID_NONE, T("ProfileCreatorDialog.CopyModsLabel"));
		copyOptionsSizer->Add(m_CopyModsCheckBox, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING_SMALL);

		// Copy mod tags
		m_CopyModTagsCheckBox = new KxCheckBox(canvas, KxID_NONE, T("ProfileCreatorDialog.CopyModTags"));
		copyOptionsSizer->Add(m_CopyModTagsCheckBox, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING_SMALL);

		m_NameInput->SetFocus();
		AddUserWindow(m_NameInput);
		AdjustWindow(pos);
		return true;
	}
	return false;
}

KProfileCreatorDialog::KProfileCreatorDialog(wxWindow* parent, KProfile* profileTemplate)
	:m_Template(profileTemplate)
{
	Create(parent, KxID_NONE, TF("ProfileCreatorDialog.Caption").arg(profileTemplate->GetName()), wxDefaultPosition, wxDefaultSize, KxBTN_OK|KxBTN_CANCEL, DefaultStyle|KxCBD_READONLY);

	Bind(KxEVT_STDDIALOG_BUTTON, &KProfileCreatorDialog::OnButtonClick, this);
	m_ProfilesList->Bind(wxEVT_COMBOBOX, &KProfileCreatorDialog::OnSelectConfiguration, this);

	wxCommandEvent event(wxEVT_COMBOBOX);
	event.SetInt(0);
	m_ProfilesList->HandleWindowEvent(event);
}
KProfileCreatorDialog::~KProfileCreatorDialog()
{
}

void KProfileCreatorDialog::OnSelectConfiguration(wxCommandEvent& event)
{
	bool isEnabled = event.GetSelection() != 0;

	m_CopyProfileConfigCheckBox->Enable(isEnabled);
	m_CopyRunManagerProgramsCheckBox->Enable(isEnabled);

	m_CopyGameConfigCheckBox->Enable(isEnabled && m_Template->IsVirtualConfigEnabled());
	m_CopyModsCheckBox->Enable(isEnabled && m_Template->IsVirtualModsEnabled());
	m_CopyModTagsCheckBox->Enable(isEnabled && m_Template->IsVirtualModsEnabled());
}
void KProfileCreatorDialog::OnButtonClick(wxNotifyEvent& event)
{
	if (event.GetId() == KxID_OK && !OnOK(event))
	{
		event.Veto();
	}
}
bool KProfileCreatorDialog::OnOK(wxNotifyEvent& event)
{
	m_Name = m_NameInput->GetValue();
	if (!KProfile::IsProfileIDValid(m_Name))
	{
		KLogEvent(T("ProfileCreatorDialog.ProfileNameInvalid"), KLOG_WARNING, this).Send();
		return false;
	}
	else if (m_Template->HasConfig(m_Name))
	{
		KLogEvent(T("ProfileCreatorDialog.ProfileNameCollision"), KLOG_WARNING, this).Send();
		return false;
	}
	else
	{
		wxString baseConfig;
		if (m_ProfilesList->GetSelection() != 0)
		{
			baseConfig = m_Template->GetConfigsList()[m_ProfilesList->GetSelection() - 1];
		}

		KProfileAddConfig config;
		config.CopyProfileConfig = m_CopyProfileConfigCheckBox->IsChecked();
		config.CopyGameConfig = m_CopyGameConfigCheckBox->IsChecked();
		config.CopyMods = m_CopyModsCheckBox->IsChecked();
		config.CopyModTags = m_CopyModTagsCheckBox->IsChecked();
		config.CopyRunManagerPrograms = m_CopyRunManagerProgramsCheckBox->IsChecked();

		if (!m_Template->AddConfig(m_Name, baseConfig, GetParent(), config))
		{
			KLogEvent(T("ProfileCreatorDialog.CreationError"), KLOG_ERROR, this).Send();
		}
		return true;
	}
}
