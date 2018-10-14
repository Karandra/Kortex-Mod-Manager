#pragma once
#include "stdafx.h"
#include <KxFramework/KxSplitterWindow.h>
#include <KxFramework/KxComboBoxDialog.h>
class KGameInstance;
class KxTextBox;
class KxCheckBox;
class KxComboBox;

class KInstanceCreatorDialog: public KxComboBoxDialog
{
	private:
		KGameInstance* m_InstanceTemplate = NULL;
		KxTextBox* m_NameInput = NULL;
		KxComboBox* m_InstancesList = NULL;
		
		KxCheckBox* m_CopyInstanceConfigCHK = NULL;
		KxCheckBox* m_CopyModTagsCHK = NULL;
		KxCheckBox* m_CopyProgramsCHK = NULL;
		
		wxString m_Name;

	private:
		bool Create(wxWindow* parent,
					wxWindowID id,
					const wxString& caption,
					const wxPoint & pos = wxDefaultPosition,
					const wxSize & size = wxDefaultSize,
					int buttons = DefaultButtons,
					long style = DefaultStyle
		);
	
	public:
		KInstanceCreatorDialog(wxWindow* parent, KGameInstance* instanceTemplate);
		virtual ~KInstanceCreatorDialog();

	public:
		const wxString& GetConfigID() const
		{
			return m_Name;
		}

	private:
		const KGameInstance* GetSelectedInstance() const;

		void OnSelectInstance(wxCommandEvent& event);
		void OnButtonClick(wxNotifyEvent& event);
		bool OnOK(wxNotifyEvent& event);
};
