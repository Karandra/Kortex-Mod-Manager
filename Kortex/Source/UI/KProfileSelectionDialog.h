#pragma once
#include "stdafx.h"
#include <KxFramework/KxSplitterWindow.h>
#include <KxFramework/KxComboBoxDialog.h>
#include <KxFramework/KxBitmapComboBox.h>
#include <KxFramework/KxListBox.h>
#include <KxFramework/KxTextBox.h>
class KxButton;

class KProfile;
class KProfileSelectionDialog: public KxComboBoxDialog
{
	private:
		KxSplitterWindow* m_Splitter = NULL;

		KxPanel* m_LeftPane = NULL;
		wxBoxSizer* m_LeftSizer = NULL;
		KxBitmapComboBox* m_TemplatesList = NULL;
		KxListBox* m_ProfilesList = NULL;

		KxPanel* m_RightPane = NULL;
		wxBoxSizer* m_RightSizer = NULL;
		KxTextBox* m_TextBox = NULL;

		KxButton* m_OK = NULL;
		KxButton* m_Create = NULL;
		KxButton* m_Remove = NULL;
		KxButton* m_CreateShortcut = NULL;

	private:
		wxString m_NewTemplate;
		wxString m_NewConfig;
		wxString m_NewGameRoot;

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
		KProfileSelectionDialog(wxWindow* parent);
		virtual ~KProfileSelectionDialog();

	public:
		const wxString& GetNewTemplate() const
		{
			return m_NewTemplate;
		}
		const wxString& GetNewConfig() const
		{
			return m_NewConfig;
		}
		
		bool IsNewGameRootSet() const
		{
			return !m_NewGameRoot.IsEmpty();
		}
		const wxString& GetNewGameRoot() const
		{
			return m_NewGameRoot;
		}

	private:
		KProfile* GetSelectedTemplate(int index = -1) const;
		wxString GetSelectedConfigID(const KProfile* profileTemplate, int index = -1) const;
		wxString GetSelectedConfigID() const
		{
			if (KProfile* profileTemplate = GetSelectedTemplate())
			{
				return GetSelectedConfigID(profileTemplate);
			}
			return wxEmptyString;
		}

		void Configure();
		void LoadTemplatesList();
		void LoadProfilesList(const KProfile* profile, const wxString& selectID);
		bool AskForGameFolder(KProfile* profileTemplate, const wxString& currentGamePath);

		void OnCreateShortcut(wxCommandEvent& event);
		void OnSelectProfile(wxNotifyEvent& event);
		void OnUpdateProfiles(wxNotifyEvent& event);
		void OnDisplayTemplateInfo(KProfile* profileTemplate);
};
