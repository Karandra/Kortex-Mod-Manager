#include "stdafx.h"
#include "KInstanceSelectionDialog.h"
#include "GameInstance/KGameInstance.h"
#include "UI/KInstanceCreatorDialog.h"
#include "KThemeManager.h"
#include "KApp.h"
#include "KBitmapSize.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxSplitterWindow.h>
#include <KxFramework/KxBitmapComboBox.h>
#include <KxFramework/KxListBox.h>
#include <KxFramework/KxTextBox.h>
#include <KxFramework/KxButton.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxShellLink.h>
#include <KxFramework/KxLibrary.h>

bool KInstanceSelectionDialog::Create(wxWindow* parent,
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
		m_CreateShortcut = AddButton(KxID_NONE, T("InstanceSelection.CreateShortcut"), true).As<KxButton>();
		m_Remove = AddButton(KxID_REMOVE, T("InstanceSelection.RemoveInstance"), true).As<KxButton>();
		m_Create = AddButton(KxID_ADD, T("InstanceSelection.CreateInstance"), true).As<KxButton>();

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

		m_InstancesList = new KxListBox(m_LeftPane, KxID_NONE);
		m_LeftSizer->Add(m_InstancesList, 1, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING);
		AddUserWindow(m_InstancesList);

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

		m_CreateShortcut->Bind(wxEVT_BUTTON, &KInstanceSelectionDialog::OnCreateShortcut, this);
		return true;
	}
	return false;
}

KInstanceSelectionDialog::KInstanceSelectionDialog(wxWindow* parent)
{
	Create(parent, KxID_NONE, T("InstanceSelection.Caption"), wxDefaultPosition, wxDefaultSize, KxBTN_OK|KxBTN_CANCEL, DefaultStyle|KxCBD_READONLY|KxCBD_BITMAP);
	Configure();
}
KInstanceSelectionDialog::~KInstanceSelectionDialog()
{
}

KGameInstance* KInstanceSelectionDialog::GetSelectedTemplate() const
{
	void* data = m_TemplatesList->GetClientData(m_TemplatesList->GetSelection());
	if (data)
	{
		return static_cast<KGameInstance*>(data);
	}
	return NULL;
}
KGameInstance* KInstanceSelectionDialog::GetSelectedInstance() const
{
	void* data = (void*)m_InstancesList->GetItemData(m_InstancesList->GetSelection());
	if (data)
	{
		return static_cast<KGameInstance*>(data);
	}
	return NULL;
}

void KInstanceSelectionDialog::Configure()
{
	Bind(KxEVT_STDDIALOG_BUTTON, &KInstanceSelectionDialog::OnButton, this);
	Bind(KEVT_PROFILE_UPDATE_LIST, &KInstanceSelectionDialog::OnUpdateProfiles, this);
	
	m_TemplatesList->Bind(wxEVT_COMBOBOX, [this](wxCommandEvent& event)
	{
		const KGameInstance* instanceTemplate = GetSelectedTemplate();
		if (instanceTemplate)
		{
			KGameInstance* activeInstance = KGameInstance::GetActive();

			LoadInstancesList(instanceTemplate, activeInstance ? activeInstance->GetInstanceID() : wxEmptyString);
			OnDisplayTemplateInfo(instanceTemplate);
		}
	});
	m_InstancesList->Bind(wxEVT_LIST_ITEM_SELECTED, [this](wxCommandEvent& event)
	{
		KGameInstance* instance = GetSelectedInstance();

		m_OK->Enable(true);
		m_Remove->Enable(!instance->IsActiveInstance());
		m_CreateShortcut->Enable(true);
		event.Skip();
	});
	m_InstancesList->Bind(wxEVT_LIST_ITEM_DESELECTED, [this](wxCommandEvent& event)
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
void KInstanceSelectionDialog::LoadTemplatesList()
{
	KGameID currentGame = KApp::Get().GetCurrentGameID();
	KBitmapSize size;
	size.FromSystemIcon();

	KxImageList* imageList = new KxImageList(size.GetWidth(), size.GetHeight(), false, KGameInstance::GetTemplatesCount());
	m_TemplatesList->AssignImageList(imageList);

	int select = 0;
	for (const auto& instanceTemplate: KGameInstance::GetTemplates())
	{
		int imageID = imageList->Add(instanceTemplate->GetIcon());
		int index = m_TemplatesList->AddItem(instanceTemplate->GetName(), imageID);
		m_TemplatesList->SetClientData(index, (void*)instanceTemplate.get());
		
		if (instanceTemplate->GetGameID() == currentGame)
		{
			select = index;
		}
	}
	m_TemplatesList->SetSelection(select);

	wxCommandEvent event(wxEVT_COMBOBOX);
	event.SetInt(select);
	m_TemplatesList->HandleWindowEvent(event);
}
void KInstanceSelectionDialog::LoadInstancesList(const KGameInstance* instanceTemplate, const wxString& selectID)
{
	m_OK->Disable();
	m_Remove->Disable();
	m_CreateShortcut->Disable();
	m_InstancesList->ClearItems();

	if (instanceTemplate)
	{
		size_t select = 0;
		for (const auto& instanceTemplate: instanceTemplate->GetInstances())
		{
			int index = m_InstancesList->AddItem(instanceTemplate->GetInstanceID());
			m_InstancesList->SetItemPtrData(index, (intptr_t)instanceTemplate.get());
			if (instanceTemplate->GetInstanceID() == selectID)
			{
				select = index;
			}
		}
		m_InstancesList->Select(select);
		m_InstancesList->SetFocus();

		wxCommandEvent event(wxEVT_LIST_ITEM_SELECTED);
		event.SetInt(select);
		m_TemplatesList->HandleWindowEvent(event);
	}
}
bool KInstanceSelectionDialog::AskForGameFolder(const KGameInstance* instanceTemplate, const wxString& currentGamePath)
{
	KxTaskDialog messageDialog(this, KxID_NONE, TF("InstanceSelection.GameNotFound.Caption").arg(instanceTemplate->GetName()), TF("InstanceSelection.GameNotFound.Message").arg(instanceTemplate->GetName()), KxBTN_CANCEL, KxICON_WARNING);
	messageDialog.AddButton(KxID_SELECT_FOLDER);
	if (messageDialog.ShowModal() == KxID_SELECT_FOLDER)
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

void KInstanceSelectionDialog::OnCreateShortcut(wxCommandEvent& event)
{
	const KGameInstance* instanceTemplate = GetSelectedTemplate();
	KGameInstance* configID = GetSelectedInstance();
	if (instanceTemplate && configID)
	{
		KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_SAVE);
		dialog.SetDefaultExtension("lnk");
		dialog.SetFileName(wxString::Format("%s - %s", instanceTemplate->GetShortName(), configID));
		dialog.SetOptionEnabled(KxFBD_NO_DEREFERENCE_LINKS);

		dialog.AddFilter("*.lnk", T("FileFilter.Shortcuts"));
		dialog.AddFilter("*", T("FileFilter.AllFiles"));

		if (dialog.ShowModal() == KxID_OK)
		{
			KxShellLink link;
			link.SetTarget(KxLibrary(NULL).GetFileName());
			link.SetArguments(wxString::Format("-GameID \"%s\" -InstanceID \"%s\"", instanceTemplate->GetGameID(), configID));
			link.SetWorkingFolder(KxFile::GetCWD());
			link.SetIconLocation(KxFile(instanceTemplate->GetIconLocation()).GetFullPath());
			link.Save(dialog.GetResult());
		}
	}
}
void KInstanceSelectionDialog::OnButton(wxNotifyEvent& event)
{
	wxWindowID id = event.GetId();
	KGameInstance* instanceTemplate = GetSelectedTemplate();
	KGameInstance* instance = GetSelectedInstance();

	if (id == KxID_ADD || id == KxID_REMOVE)
	{
		event.Veto();
		if (instanceTemplate)
		{
			if (id == KxID_ADD)
			{
				KInstanceCreatorDialog dialog(this, instanceTemplate);
				if (dialog.ShowModal() == KxID_OK)
				{
					LoadInstancesList(instanceTemplate, dialog.GetName());
				}
			}
			else if (id == KxID_REMOVE && instance)
			{
				wxWindowID ret = KxTaskDialog(
					this,
					KxID_NONE,
					TF("InstanceSelection.ConfirmRemoving.Caption").arg(instance->GetInstanceID()),
					TF("InstanceSelection.ConfirmRemoving.Message").arg(instance->GetInstanceDir()).arg(instance->GetInstanceID()),
					KxBTN_YES|KxBTN_NO, KxICON_WARNING
				).ShowModal();
				if (ret == KxID_YES)
				{
					if (instance->WithdrawDeploy())
					{
						instance = NULL;
						LoadInstancesList(instanceTemplate, wxEmptyString);
					}
					else
					{
						KLogEvent(T("InstanceCreatorDialog.DeletionError"), KLOG_ERROR).Send();
					}
				}
			}
		}
	}
	else if (id == KxID_OK && instanceTemplate)
	{
		wxString gamePath = instance->GetGameDir();
		
		// Try defaults
		if (gamePath.IsEmpty())
		{
			gamePath = instanceTemplate->GetGameDir();
		}

		// Ask user
		if (!KxFile(gamePath).IsFolderExist())
		{
			if (!AskForGameFolder(instanceTemplate, gamePath))
			{
				return;
			}
		}

		m_NewGameID = instanceTemplate->GetGameID();
		m_NewInstance = instance->GetInstanceID();

		event.Skip();
	}
}
void KInstanceSelectionDialog::OnUpdateProfiles(wxNotifyEvent& event)
{
	LoadInstancesList(GetSelectedTemplate(), event.GetString());
}
void KInstanceSelectionDialog::OnDisplayTemplateInfo(const KGameInstance* instanceTemplate)
{
	m_TextBox->Clear();
	if (instanceTemplate)
	{
		m_TextBox->SetValue(instanceTemplate->GetName());
	}
}
