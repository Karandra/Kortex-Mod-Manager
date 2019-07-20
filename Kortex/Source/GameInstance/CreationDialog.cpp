#include "stdafx.h"
#include "CreationDialog.h"
#include "GameInstance/IGameInstance.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxComboBox.h>
#include <KxFramework/KxTextBox.h>
#include <KxFramework/KxCheckBox.h>

namespace Kortex::GameInstance
{
	wxWindow* CreationDialog::GetDialogFocusCtrl() const
	{
		return m_NameInput;
	}

	bool CreationDialog::Create(wxWindow* parent, const GameID& gameID)
	{
		m_InstanceTemplate = IGameInstance::GetTemplate(gameID);
		if (KxComboBoxDialog::Create(parent, KxID_NONE, KTrf("InstanceCreatorDialog.Caption", m_InstanceTemplate->GetGameName()), wxDefaultPosition, wxDefaultSize, KxBTN_OK|KxBTN_CANCEL, DefaultStyle|KxCBD_READONLY))
		{
			Bind(KxEVT_STDDIALOG_BUTTON, &CreationDialog::OnButtonClick, this);

			SetMainIcon(KxICON_NONE);
			SetWindowResizeSide((wxOrientation)0);
			wxWindow* contentPanel = GetContentWindow();
			wxSizer* mainSizer = GetContentWindowSizer();
			m_InstancesList = static_cast<KxComboBox*>(GetDialogMainCtrl());

			// Name
			wxBoxSizer* nameSizer = new wxBoxSizer(wxHORIZONTAL);
			mainSizer->Add(nameSizer, 0, wxEXPAND);

			KxLabel* nameLabel = new KxLabel(contentPanel, KxID_NONE, KTr("InstanceCreatorDialog.NameLabel"));
			nameSizer->Add(nameLabel, 1);

			m_NameInput = new KxTextBox(contentPanel, KxID_NONE);
			nameSizer->Add(m_NameInput, 1);

			// Copy from
			wxBoxSizer* copySizer = new wxBoxSizer(wxHORIZONTAL);
			mainSizer->Add(copySizer, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING_SMALL);

			KxLabel* copyFromLabel = new KxLabel(contentPanel, KxID_NONE, KTr("InstanceCreatorDialog.CopyFrom"));
			copySizer->Add(copyFromLabel, 1, wxTOP, KLC_VERTICAL_SPACING_SMALL);

			wxBoxSizer* copyOptionsSizer = new wxBoxSizer(wxVERTICAL);
			copySizer->Add(copyOptionsSizer, 1, wxEXPAND);

			mainSizer->Detach(m_InstancesList);
			copyOptionsSizer->Add(m_InstancesList, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING_SMALL);
			m_InstancesList->AddItem(KVarExp("<$T(ID_NONE)>"));

			for (const auto& instance: IGameInstance::GetShallowInstances())
			{
				if (instance->GetGameID() == m_InstanceTemplate->GetGameID())
				{
					int index = m_InstancesList->AddItem(instance->GetInstanceID());
					m_InstancesList->SetClientData(index, (void*)instance.get());
				}
			}
			m_InstancesList->SetSelection(0);
			m_InstancesList->Bind(wxEVT_COMBOBOX, &CreationDialog::OnSelectInstance, this);

			// Copy instance config
			m_CopyInstanceConfigCHK = new KxCheckBox(contentPanel, KxID_NONE, KTr("InstanceCreatorDialog.CopyInstanceConfig"));
			copyOptionsSizer->Add(m_CopyInstanceConfigCHK, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING_SMALL);

			AddUserWindow(m_NameInput);
			AdjustWindow(wxDefaultPosition);

			// Call event handler
			wxCommandEvent event(wxEVT_COMBOBOX);
			event.SetInt(0);
			m_InstancesList->ProcessWindowEvent(event);
			return true;
		}
		return false;
	}

	const IGameInstance* CreationDialog::GetSelectedInstance() const
	{
		return static_cast<const IGameInstance*>(m_InstancesList->GetClientData(m_InstancesList->GetSelection()));
	}

	void CreationDialog::OnSelectInstance(wxCommandEvent& event)
	{
		bool isEnabled = event.GetSelection() != 0;
		m_CopyInstanceConfigCHK->Enable(isEnabled);
	}
	void CreationDialog::OnButtonClick(wxNotifyEvent& event)
	{
		if (event.GetId() == KxID_OK && !OnOK(event))
		{
			event.Veto();
		}
	}
	bool CreationDialog::OnOK(wxNotifyEvent& event)
	{
		m_InstanceID = m_NameInput->GetValue();
		if (!IGameInstance::IsValidInstanceID(m_InstanceID))
		{
			BroadcastProcessor::Get().ProcessEvent(LogEvent::EvtWarning, KTr("InstanceCreatorDialog.InstanceNameInvalid"));
			return false;
		}
		else if (IGameInstance::GetShallowInstance(m_InstanceID) != nullptr)
		{
			BroadcastProcessor::Get().ProcessEvent(LogEvent::EvtWarning, KTr("InstanceCreatorDialog.InstanceNameCollision"));
			return false;
		}
		else
		{
			if (IGameInstance* newInstance = m_InstanceTemplate->NewShallowInstance(m_InstanceID, m_InstanceTemplate->GetGameID()))
			{
				uint32_t options = 0;
				const IGameInstance* baseInstance = GetSelectedInstance();
				if (baseInstance)
				{
					if (m_CopyInstanceConfigCHK->IsChecked())
					{
						options |= GameInstance::CopyOptionsInstance::Config;
					}
				}

				if (newInstance->Deploy(baseInstance, options))
				{
					return true;
				}
			}

			BroadcastProcessor::Get().ProcessEvent(LogEvent::EvtError, KTr("InstanceCreatorDialog.CreationError"));
			return false;
		}
	}
}
