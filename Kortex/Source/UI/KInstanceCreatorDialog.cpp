#include "stdafx.h"
#include "KInstanceCreatorDialog.h"
#include "ProgramManager/KProgramManager.h"
#include "Events/LogEvent.h"
#include "GameInstance/IGameInstance.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxTaskDialog.h>
#include <KxFramework/KxComboBox.h>
#include <KxFramework/KxTextBox.h>
#include <KxFramework/KxCheckBox.h>

namespace Kortex
{
	bool KInstanceCreatorDialog::Create(wxWindow* parent,
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
			m_InstancesList = static_cast<KxComboBox*>(GetDialogMainCtrl());

			// Name
			wxBoxSizer* nameSizer = new wxBoxSizer(wxHORIZONTAL);
			mainSizer->Add(nameSizer, 0, wxEXPAND);

			KxLabel* nameLabel = new KxLabel(canvas, KxID_NONE, KTr("InstanceCreatorDialog.NameLabel"));
			nameSizer->Add(nameLabel, 1);

			m_NameInput = new KxTextBox(canvas, KxID_NONE);
			nameSizer->Add(m_NameInput, 1);

			// Copy from
			wxBoxSizer* copySizer = new wxBoxSizer(wxHORIZONTAL);
			mainSizer->Add(copySizer, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING_SMALL);

			KxLabel* copyFromLabel = new KxLabel(canvas, KxID_NONE, KTr("InstanceCreatorDialog.CopyFrom"));
			copySizer->Add(copyFromLabel, 1, wxTOP, KLC_VERTICAL_SPACING_SMALL);

			wxBoxSizer* copyOptionsSizer = new wxBoxSizer(wxVERTICAL);
			copySizer->Add(copyOptionsSizer, 1, wxEXPAND);

			mainSizer->Detach(m_InstancesList);
			copyOptionsSizer->Add(m_InstancesList, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING_SMALL);
			m_InstancesList->AddItem(KVarExp("<$T(ID_NONE)>"));

			for (const auto& instance: m_InstanceTemplate->GetActiveInstances())
			{
				int index = m_InstancesList->AddItem(instance->GetInstanceID());
				m_InstancesList->SetClientData(index, (void*)instance.get());
			}
			m_InstancesList->SetSelection(0);

			// Copy instance config
			m_CopyInstanceConfigCHK = new KxCheckBox(canvas, KxID_NONE, KTr("InstanceCreatorDialog.CopyInstanceConfig"));
			copyOptionsSizer->Add(m_CopyInstanceConfigCHK, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING_SMALL);

			// Copy mod tags
			m_CopyModTagsCHK = new KxCheckBox(canvas, KxID_NONE, KTr("InstanceCreatorDialog.CopyModTags"));
			copyOptionsSizer->Add(m_CopyModTagsCHK, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING_SMALL);

			// Copy programs
			m_CopyProgramsCHK = new KxCheckBox(canvas, KxID_NONE, KTr("InstanceCreatorDialog.CopyPrograms"));
			copyOptionsSizer->Add(m_CopyProgramsCHK, 0, wxEXPAND|wxTOP, KLC_VERTICAL_SPACING_SMALL);

			m_NameInput->SetFocus();
			AddUserWindow(m_NameInput);
			AdjustWindow(pos);
			return true;
		}
		return false;
	}

	KInstanceCreatorDialog::KInstanceCreatorDialog(wxWindow* parent, IGameInstance* instanceTemplate)
		:m_InstanceTemplate(instanceTemplate)
	{
		Create(parent, KxID_NONE, KTrf("InstanceCreatorDialog.Caption", instanceTemplate->GetName()), wxDefaultPosition, wxDefaultSize, KxBTN_OK|KxBTN_CANCEL, DefaultStyle|KxCBD_READONLY);

		Bind(KxEVT_STDDIALOG_BUTTON, &KInstanceCreatorDialog::OnButtonClick, this);
		m_InstancesList->Bind(wxEVT_COMBOBOX, &KInstanceCreatorDialog::OnSelectInstance, this);

		wxCommandEvent event(wxEVT_COMBOBOX);
		event.SetInt(0);
		m_InstancesList->HandleWindowEvent(event);
	}
	KInstanceCreatorDialog::~KInstanceCreatorDialog()
	{
	}

	const IGameInstance* KInstanceCreatorDialog::GetSelectedInstance() const
	{
		return static_cast<IGameInstance*>(m_InstancesList->GetClientData(m_InstancesList->GetSelection()));
	}

	void KInstanceCreatorDialog::OnSelectInstance(wxCommandEvent& event)
	{
		bool isEnabled = event.GetSelection() != 0;

		m_CopyInstanceConfigCHK->Enable(isEnabled);
		m_CopyModTagsCHK->Enable(isEnabled);
		m_CopyProgramsCHK->Enable(isEnabled);
	}
	void KInstanceCreatorDialog::OnButtonClick(wxNotifyEvent& event)
	{
		if (event.GetId() == KxID_OK && !OnOK(event))
		{
			event.Veto();
		}
	}
	bool KInstanceCreatorDialog::OnOK(wxNotifyEvent& event)
	{
		m_Name = m_NameInput->GetValue();
		if (!IGameInstance::IsValidInstanceID(m_Name))
		{
			LogEvent(KTr("InstanceCreatorDialog.InstanceNameInvalid"), LogLevel::Warning, this).Send();
			return false;
		}
		else if (m_InstanceTemplate->HasInstance(m_Name))
		{
			LogEvent(KTr("InstanceCreatorDialog.InstanceNameCollision"), LogLevel::Warning, this).Send();
			return false;
		}
		else
		{
			if (IGameInstance* newInstance = m_InstanceTemplate->AddInstance(m_Name))
			{
				uint32_t options = 0;
				const IGameInstance* baseInstance = GetSelectedInstance();
				if (baseInstance)
				{
					if (m_CopyInstanceConfigCHK->IsChecked())
					{
						options |= GameInstance::CopyOptionsInstance::Config;
					}
					if (m_CopyModTagsCHK->IsChecked())
					{
						options |= GameInstance::CopyOptionsInstance::ModTags;
					}
					if (m_CopyProgramsCHK->IsChecked())
					{
						options |= GameInstance::CopyOptionsInstance::Programs;
					}
				}

				if (newInstance->Deploy(baseInstance, options))
				{
					return true;
				}
			}

			LogEvent(KTr("InstanceCreatorDialog.CreationError"), LogLevel::Error, this).Send();
			return false;
		}
	}
}
