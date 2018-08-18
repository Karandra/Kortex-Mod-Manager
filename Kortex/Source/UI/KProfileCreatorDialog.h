#pragma once
#include "stdafx.h"
#include <KxFramework/KxSplitterWindow.h>
#include <KxFramework/KxComboBoxDialog.h>
#include <KxFramework/KxComboBox.h>
#include <KxFramework/KxTextBox.h>
#include <KxFramework/KxCheckBox.h>

class KProfile;
class KProfileCreatorDialog: public KxComboBoxDialog
{
	private:
		KProfile* m_Template = NULL;
		KxTextBox* m_NameInput = NULL;
		KxComboBox* m_ProfilesList = NULL;
		
		KxCheckBox* m_CopyProfileConfigCheckBox = NULL;
		KxCheckBox* m_CopyRunManagerProgramsCheckBox = NULL;
		KxCheckBox* m_CopyGameConfigCheckBox = NULL;
		KxCheckBox* m_CopyModsCheckBox = NULL;
		KxCheckBox* m_CopyModTagsCheckBox = NULL;
		
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
		KProfileCreatorDialog(wxWindow* parent, KProfile* profileTemplate);
		virtual ~KProfileCreatorDialog();

	public:
		const wxString& GetConfigID() const
		{
			return m_Name;
		}

	private:
		void OnSelectConfiguration(wxCommandEvent& event);
		void OnButtonClick(wxNotifyEvent& event);
		bool OnOK(wxNotifyEvent& event);
};
