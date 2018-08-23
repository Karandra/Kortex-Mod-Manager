#include "stdafx.h"
#include "KProfileSelectionDialog.h"
#include "Profile/KProfile.h"
#include "UI/KProfileCreatorDialog.h"
#include "KThemeManager.h"
#include "KApp.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxButton.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxShellLink.h>
#include <KxFramework/KxLibrary.h>

bool KProfileSelectionDialog::Create(wxWindow* parent,
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
		SetWindowResizeSide(wxBOTH);

		// Bottom panel
		m_OK = GetButton(KxID_OK).As<KxButton>();
		m_CreateShortcut = AddButton(KxID_NONE, T("ProfileSelection.CreateShortcut"), true).As<KxButton>();
		m_Remove = AddButton(KxID_REMOVE, T("ProfileSelection.RemoveProfile"), true).As<KxButton>();
		m_Create = AddButton(KxID_ADD, T("ProfileSelection.CreateProfile"), true).As<KxButton>();

		// Splitter
		wxSizer* mainSizer = GetContentWindowSizer();
		m_Splitter = new KxSplitterWindow(GetContentWindow(), KxID_NONE);
		KThemeManager::Get().ProcessWindow(m_Splitter);
		m_Splitter->SetMinimumPaneSize(200);

		// Left pane
		m_LeftPane = new KxPanel(m_Splitter, KxID_NONE);
		m_LeftSizer = new wxBoxSizer(wxVERTICAL);
		m_LeftPane->SetSizer(m_LeftSizer);

		mainSizer->Detach(GetDialogMainCtrl());
		GetDialogMainCtrl()->Reparent(m_LeftPane);
		m_LeftSizer->Add(GetDialogMainCtrl(), 0, wxEXPAND);
		m_TemplatesList = static_cast<KxBitmapComboBox*>(GetDialogMainCtrl());

		m_ProfilesList = new KxListBox(m_LeftPane, KxID_NONE);
		m_LeftSizer->Add(m_ProfilesList, 1, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);
		AddUserWindow(m_ProfilesList);

		// Right pane
		m_RightPane = new KxPanel(m_Splitter, KxID_NONE);
		m_RightSizer = new wxBoxSizer(wxVERTICAL);
		m_RightPane->SetSizer(m_RightSizer);

		m_TextBox = new KxTextBox(m_RightPane, KxID_NONE, wxEmptyString, KxTextBox::DefaultStyle|wxTE_MULTILINE);
		m_TextBox->SetEditable(false);
		m_RightSizer->Add(m_TextBox, 1, wxEXPAND);
		AddUserWindow(m_TextBox);

		// Done
		m_Splitter->SplitVertically(m_LeftPane, m_RightPane, 300);
		mainSizer->Add(m_Splitter, 1, wxEXPAND);

		m_CreateShortcut->Bind(wxEVT_BUTTON, &KProfileSelectionDialog::OnCreateShortcut, this);
		return true;
	}
	return false;
}

KProfileSelectionDialog::KProfileSelectionDialog(wxWindow* parent)
{
	Create(parent, KxID_NONE, T("ProfileSelection.Caption"), wxDefaultPosition, wxDefaultSize, KxBTN_OK|KxBTN_CANCEL, DefaultStyle|KxCBD_READONLY|KxCBD_BITMAP);
	Configure();
}
KProfileSelectionDialog::~KProfileSelectionDialog()
{
}

KProfile* KProfileSelectionDialog::GetSelectedTemplate(int index) const
{
	void* data = m_TemplatesList->GetClientData(index == -1 ? m_TemplatesList->GetSelection() : index);
	if (data)
	{
		return static_cast<KProfile*>(data);
	}
	return NULL;
}
wxString KProfileSelectionDialog::GetSelectedConfigID(const KProfile* profileTemplate, int index) const
{
	if (profileTemplate)
	{
		index = index == -1 ? m_ProfilesList->GetSelection() : index;
		return profileTemplate->GetConfigsList()[index];
	}
	return wxEmptyString;
}

void KProfileSelectionDialog::Configure()
{
	Bind(KxEVT_STDDIALOG_BUTTON, &KProfileSelectionDialog::OnSelectProfile, this);
	Bind(KEVT_UPDATE_PROFILES, &KProfileSelectionDialog::OnUpdateProfiles, this);
	
	m_TemplatesList->Bind(wxEVT_COMBOBOX, [this](wxCommandEvent& event)
	{
		KProfile* profileTemplate = GetSelectedTemplate(event.GetInt());
		LoadProfilesList(profileTemplate, KApp::Get().GetCurrentConfigID());
		OnDisplayTemplateInfo(profileTemplate);
	});
	m_ProfilesList->Bind(wxEVT_LIST_ITEM_SELECTED, [this](wxCommandEvent& event)
	{
		m_OK->Enable(true);
		m_Remove->Enable(GetSelectedConfigID() != KApp::Get().GetCurrentConfigID());
		m_CreateShortcut->Enable(true);
		event.Skip();
	});
	m_ProfilesList->Bind(wxEVT_LIST_ITEM_DESELECTED, [this](wxCommandEvent& event)
	{
		m_OK->Enable(false);
		m_Remove->Enable(false);
		m_CreateShortcut->Enable(false);
		event.Skip();
	});

	m_OK->Disable();
	m_Remove->Disable();
	m_CreateShortcut->Disable();
	LoadTemplatesList();
	AdjustWindow(wxDefaultPosition, wxSize(650, 400));
}
void KProfileSelectionDialog::LoadTemplatesList()
{
	wxString templateID = KApp::Get().GetCurrentTemplateID();

	auto list = KProfile::GetTemplatesList();
	KxImageList* pImageList = new KxImageList(32, 32, false, list.size());
	m_TemplatesList->AssignImageList(pImageList);

	int select = 0;
	for (const KProfile* profileTemplate: list)
	{
		int imageID = pImageList->Add(profileTemplate->GetIcon().Rescale(32, 32, wxIMAGE_QUALITY_HIGH));
		int index = m_TemplatesList->AddItem(profileTemplate->GetName(), imageID);
		m_TemplatesList->SetClientData(index, (void*)profileTemplate);

		if (profileTemplate->GetID() == templateID)
		{
			select = index;
		}
	}
	m_TemplatesList->SetSelection(select);

	wxCommandEvent event(wxEVT_COMBOBOX);
	event.SetInt(select);
	m_TemplatesList->HandleWindowEvent(event);
}
void KProfileSelectionDialog::LoadProfilesList(const KProfile* profile, const wxString& selectID)
{
	m_OK->Disable();
	m_Remove->Disable();
	m_CreateShortcut->Disable();
	m_ProfilesList->ClearItems();

	if (profile)
	{
		size_t select = 0;
		const KxStringVector& list = profile->GetConfigsList();
		for (size_t i = 0; i < list.size(); i++)
		{
			m_ProfilesList->AddItem(list[i]);
			if (list[i] == selectID)
			{
				select = i;
			}
		}

		m_ProfilesList->Select(select);
		m_ProfilesList->SetFocus();

		wxCommandEvent event(wxEVT_LIST_ITEM_SELECTED);
		event.SetInt(select);
		m_TemplatesList->HandleWindowEvent(event);
	}
}
bool KProfileSelectionDialog::AskForGameFolder(KProfile* profileTemplate, const wxString& currentGamePath)
{
	KxTaskDialog dialog(this, KxID_NONE, T(profileTemplate, "ProfileSelection.GameNotFound.Caption"), T(profileTemplate, "ProfileSelection.GameNotFound.Message"), KxBTN_CANCEL, KxICON_WARNING);
	dialog.AddButton(KxID_SELECT_FOLDER);
	if (dialog.ShowModal() == KxID_SELECT_FOLDER)
	{
		KxFileBrowseDialog browseDialog(this, KxID_NONE, KxFBD_OPEN_FOLDER);
		browseDialog.SetFolder(currentGamePath);
		if (browseDialog.ShowModal())
		{
			m_NewGameRoot = browseDialog.GetResult();
			return true;
		}
	}

	m_NewGameRoot.Clear();
	return false;
}

void KProfileSelectionDialog::OnCreateShortcut(wxCommandEvent& event)
{
	KProfile* profileTemplate = GetSelectedTemplate();
	if (profileTemplate)
	{
		wxString configID = GetSelectedConfigID(profileTemplate);
		if (!configID.IsEmpty())
		{
			KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_SAVE);
			dialog.SetDefaultExtension("lnk");
			dialog.SetFileName(wxString::Format("%s - %s", profileTemplate->GetShortName(), configID));
			dialog.SetOptionEnabled(KxFBD_NO_DEREFERENCE_LINKS);

			dialog.AddFilter("*.lnk", T("FileFilter.Shortcuts"));
			dialog.AddFilter("*", T("FileFilter.AllFiles"));

			if (dialog.ShowModal() == KxID_OK)
			{
				KxShellLink link;
				link.SetTarget(KxLibrary(NULL).GetFileName());
				link.SetArguments(wxString::Format("-ProfileID \"%s\" -ConfigID \"%s\"", profileTemplate->GetID(), configID));
				link.SetWorkingFolder(KxFile::GetCWD());
				link.SetIconLocation(KxFile(profileTemplate->GetIconPath()).GetFullPath());
				link.Save(dialog.GetResult());
			}
		}
	}
}
void KProfileSelectionDialog::OnSelectProfile(wxNotifyEvent& event)
{
	wxWindowID id = event.GetId();
	KProfile* profileTemplate = GetSelectedTemplate();

	if (id == KxID_ADD || id == KxID_REMOVE)
	{
		event.Veto();
		if (profileTemplate)
		{
			if (id == KxID_ADD)
			{
				KProfileCreatorDialog dialog(this, profileTemplate);
				if (dialog.ShowModal() == KxID_OK)
				{
					LoadProfilesList(profileTemplate, dialog.GetName());
				}
			}
			else if (id == KxID_REMOVE)
			{
				const wxString& configID = profileTemplate->GetConfigsList()[m_ProfilesList->GetSelection()];
				wxWindowID ret = KxTaskDialog(
					this,
					KxID_NONE,
					TF("ProfileSelection.ConfirmRemoving.Caption").arg(configID),
					TF("ProfileSelection.ConfirmRemoving.Message").arg(profileTemplate->GetDataPath(profileTemplate->GetID())).arg(configID),
					KxBTN_YES|KxBTN_NO, KxICON_WARNING
				).ShowModal();
				if (ret == KxID_YES)
				{
					profileTemplate->RemoveConfig(configID);
					LoadProfilesList(profileTemplate, configID);
				}
			}
		}
	}
	else if (id == KxID_OK && profileTemplate)
	{
		wxString sGamePath = KProfile::GetGameRootPath(profileTemplate->GetID(), GetSelectedConfigID(profileTemplate));
		if (sGamePath.IsEmpty())
		{
			sGamePath = profileTemplate->ExpandVariables(KVAR(KVAR_GAME_ROOT));
		}

		if (sGamePath.IsEmpty() || !KxFile(sGamePath).IsFolderExist())
		{
			if (!AskForGameFolder(profileTemplate, sGamePath))
			{
				return;
			}
		}

		m_NewTemplate = profileTemplate->GetID();
		m_NewConfig = GetSelectedConfigID(profileTemplate);

		event.Skip();
	}
}
void KProfileSelectionDialog::OnUpdateProfiles(wxNotifyEvent& event)
{
	LoadProfilesList(GetSelectedTemplate(), event.GetString());
}
void KProfileSelectionDialog::OnDisplayTemplateInfo(KProfile* profileTemplate)
{
	m_TextBox->Clear();
	if (profileTemplate)
	{
		m_TextBox->SetValue(profileTemplate->GetName());
	}
}
