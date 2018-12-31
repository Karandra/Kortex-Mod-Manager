#include "stdafx.h"
#include "KInstanceSelectionDialog.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/Events.hpp>
#include "UI/KInstanceCreatorDialog.h"
#include "Utility/KBitmapSize.h"
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxSplitterWindow.h>
#include <KxFramework/KxBitmapComboBox.h>
#include <KxFramework/KxListBox.h>
#include <KxFramework/KxTextBox.h>
#include <KxFramework/KxButton.h>
#include <KxFramework/KxFile.h>
#include <KxFramework/KxFileBrowseDialog.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxShellLink.h>
#include <KxFramework/KxLibrary.h>
#include <KxFramework/KxSystem.h>

namespace Kortex
{
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
			m_CreateShortcut = AddButton(KxID_NONE, KTr("InstanceSelection.CreateShortcut"), true).As<KxButton>();
			m_Remove = AddButton(KxID_REMOVE, KTr("InstanceSelection.RemoveInstance"), true).As<KxButton>();
			m_Create = AddButton(KxID_ADD, KTr("InstanceSelection.CreateInstance"), true).As<KxButton>();

			// Splitter
			wxSizer* mainSizer = GetContentWindowSizer();
			m_Splitter = new KxSplitterWindow(GetContentWindow(), KxID_NONE);
			IThemeManager::GetActive().ProcessWindow(m_Splitter);
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
		Create(parent, KxID_NONE, KTr("InstanceSelection.Caption"), wxDefaultPosition, wxDefaultSize, KxBTN_OK|KxBTN_CANCEL, DefaultStyle|KxCBD_READONLY|KxCBD_BITMAP);
		Configure();
	}
	KInstanceSelectionDialog::~KInstanceSelectionDialog()
	{
	}

	IGameInstance* KInstanceSelectionDialog::GetSelectedTemplate() const
	{
		void* data = m_TemplatesList->GetClientData(m_TemplatesList->GetSelection());
		if (data)
		{
			return static_cast<IGameInstance*>(data);
		}
		return nullptr;
	}
	IGameInstance* KInstanceSelectionDialog::GetSelectedInstance() const
	{
		void* data = (void*)m_InstancesList->GetItemData(m_InstancesList->GetSelection());
		if (data)
		{
			return static_cast<IGameInstance*>(data);
		}
		return nullptr;
	}

	void KInstanceSelectionDialog::Configure()
	{
		Bind(KxEVT_STDDIALOG_BUTTON, &KInstanceSelectionDialog::OnButton, this);
		Bind(Kortex::Events::ProfileRefreshList, &KInstanceSelectionDialog::OnUpdateProfiles, this);

		m_TemplatesList->Bind(wxEVT_COMBOBOX, [this](wxCommandEvent& event)
		{
			const IGameInstance* instanceTemplate = GetSelectedTemplate();
			if (instanceTemplate)
			{
				IGameInstance* activeInstance = IGameInstance::GetActive();

				LoadInstancesList(instanceTemplate, activeInstance ? activeInstance->GetInstanceID() : wxEmptyString);
			}
		});
		m_InstancesList->Bind(wxEVT_LIST_ITEM_SELECTED, [this](wxCommandEvent& event)
		{
			IGameInstance* instance = GetSelectedInstance();

			m_OK->Enable(true);
			m_Remove->Enable(!instance->IsActiveInstance());
			m_CreateShortcut->Enable(true);

			OnDisplayInstanceInfo(instance);
			event.Skip();
		});
		m_InstancesList->Bind(wxEVT_LIST_ITEM_DESELECTED, [this](wxCommandEvent& event)
		{
			m_OK->Enable(false);
			m_Remove->Enable(false);
			m_CreateShortcut->Enable(false);

			OnDisplayInstanceInfo(nullptr);
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
		GameID currentGame;
		if (const IGameInstance* activeInstance = IGameInstance::GetActive())
		{
			currentGame = activeInstance->GetGameID();
		}
		else
		{
			currentGame = IApplication::GetInstance()->GetStartupGameID();
		}

		KBitmapSize size;
		size.FromSystemIcon();

		KxImageList* imageList = new KxImageList(size.GetWidth(), size.GetHeight(), false, IGameInstance::GetTemplatesCount());
		m_TemplatesList->AssignImageList(imageList);

		int select = 0;
		for (const auto& instanceTemplate: IGameInstance::GetTemplates())
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
	void KInstanceSelectionDialog::LoadInstancesList(const IGameInstance* instanceTemplate, const wxString& selectID)
	{
		m_OK->Disable();
		m_Remove->Disable();
		m_CreateShortcut->Disable();
		m_InstancesList->ClearItems();

		if (instanceTemplate)
		{
			size_t select = 0;
			for (const auto& instanceTemplate: instanceTemplate->GetActiveInstances())
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
	bool KInstanceSelectionDialog::AskForGameFolder(const IGameInstance* instanceTemplate, const wxString& currentGamePath)
	{
		KxTaskDialog messageDialog(this, KxID_NONE, KTrf("InstanceSelection.GameNotFound.Caption", instanceTemplate->GetName()), KTrf("InstanceSelection.GameNotFound.Message", instanceTemplate->GetName()), KxBTN_CANCEL, KxICON_WARNING);
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
		const IGameInstance* instanceTemplate = GetSelectedTemplate();
		IGameInstance* instance = GetSelectedInstance();
		if (instanceTemplate && instance)
		{
			KxFileBrowseDialog dialog(this, KxID_NONE, KxFBD_SAVE);
			dialog.SetDefaultExtension("lnk");
			dialog.SetFileName(wxString::Format("%s - %s", instance->GetShortName(), instance->GetInstanceID()));
			dialog.SetOptionEnabled(KxFBD_NO_DEREFERENCE_LINKS);

			dialog.AddFilter("*.lnk", KTr("FileFilter.Shortcuts"));
			dialog.AddFilter("*", KTr("FileFilter.AllFiles"));

			if (dialog.ShowModal() == KxID_OK)
			{
				KxShellLink link;
				link.SetTarget(KxLibrary(nullptr).GetFileName());
				link.SetArguments(KxFormat("-GameID \"%1\" -InstanceID \"%2\"").arg(instanceTemplate->GetGameID()).arg(instance->GetInstanceID()));
				link.SetWorkingFolder(KxFile::GetCWD());
				link.SetIconLocation(KxFile(instanceTemplate->GetIconLocation()).GetFullPath());
				link.Save(dialog.GetResult());
			}
		}
	}
	void KInstanceSelectionDialog::OnButton(wxNotifyEvent& event)
	{
		wxWindowID id = event.GetId();
		IGameInstance* instanceTemplate = GetSelectedTemplate();
		IGameInstance* instance = GetSelectedInstance();

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
						KTrf("InstanceSelection.ConfirmRemoving.Caption", instance->GetInstanceID()),
						KTrf("InstanceSelection.ConfirmRemoving.Message", instance->GetInstanceDir(), instance->GetInstanceID()),
						KxBTN_YES|KxBTN_NO, KxICON_WARNING
					).ShowModal();
					if (ret == KxID_YES)
					{
						if (instance->WithdrawDeploy())
						{
							instance = nullptr;
							LoadInstancesList(instanceTemplate, wxEmptyString);
						}
						else
						{
							LogEvent(KTr("InstanceCreatorDialog.DeletionError"), LogLevel::Error).Send();
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
				if (AskForGameFolder(instanceTemplate, gamePath))
				{
					instance->GetVariables().SetVariable(Variables::KVAR_ACTUAL_GAME_DIR, m_NewGameRoot);

					IConfigurableGameInstance* configurableInstance = nullptr;
					if (instance->QueryInterface(configurableInstance))
					{
						configurableInstance->SaveConfig();
					}
				}
				else
				{
					return;
				}
			}

			m_NewInstance = instance;
			m_NewGameID = instance->GetGameID();
			m_NewInstanceID = instance->GetInstanceID();

			event.Skip();
		}
	}
	void KInstanceSelectionDialog::OnUpdateProfiles(wxNotifyEvent& event)
	{
		LoadInstancesList(GetSelectedTemplate(), event.GetString());
	}
	void KInstanceSelectionDialog::OnDisplayInstanceInfo(const IGameInstance* instance)
	{
		auto PrintSeparator = [this]()
		{
			*m_TextBox << wxS("\r\n\r\n");
		};
		auto PrintVariables = [this](const IVariableTable& variables)
		{
			variables.Accept([this](const wxString& name, const VariableValue& value)
			{
				*m_TextBox << KxFormat(wxS("$(%1)%2 = \"%3\"")).arg(name).arg(value.IsOverride() ? wxS("*") : wxS("")).arg(value) << wxS("\r\n");
				return true;
			});
		};

		wxWindowUpdateLocker lock(m_TextBox);
		m_TextBox->Clear();
		m_TextBox->Disable();

		if (instance)
		{
			if (instance->IsActiveInstance())
			{
				instance = IGameInstance::GetActive();
			}
			m_TextBox->Enable();

			// Instance variables
			PrintVariables(instance->GetVariables());
			PrintSeparator();

			// App variables
			PrintVariables(IApplication::GetInstance()->GetVariables());
			PrintSeparator();

			// $SHF(*) variables
			for (const auto&[id, item]: KxShell::GetShellFolderList())
			{
				// Skip 'SHF_' part for 'item.second'
				*m_TextBox << KxFormat(wxS("$SHF(%1) = \"%2\"")).arg(item.second + 4).arg(KxShell::GetFolder(id)) << wxS("\r\n");
			}
			PrintSeparator();

			// $ENV(*) variables
			for (const auto&[name, value]: KxSystem::GetEnvironmentVariables())
			{
				*m_TextBox << KxFormat(wxS("$ENV(%1) = \"%2\"")).arg(name).arg(value) << wxS("\r\n");
			}
		}
	}
}
