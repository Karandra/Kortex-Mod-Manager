#pragma once
#include "stdafx.h"
#include "GameInstance/KGameID.h"
#include <KxFramework/KxComboBoxDialog.h>
class KGameInstance;
class KGameInstance;
class KxPanel;
class KxButton;
class KxTextBox;
class KxListBox;
class KxBitmapComboBox;
class KxSplitterWindow;

class KInstanceSelectionDialog: public KxComboBoxDialog
{
	private:
		KxSplitterWindow* m_Splitter = NULL;

		KxPanel* m_LeftPane = NULL;
		wxBoxSizer* m_LeftSizer = NULL;
		KxBitmapComboBox* m_TemplatesList = NULL;
		KxListBox* m_InstancesList = NULL;

		KxPanel* m_RightPane = NULL;
		wxBoxSizer* m_RightSizer = NULL;
		KxTextBox* m_TextBox = NULL;

		KxButton* m_OK = NULL;
		KxButton* m_Create = NULL;
		KxButton* m_Remove = NULL;
		KxButton* m_CreateShortcut = NULL;

	private:
		KGameID m_NewGameID;
		wxString m_NewInstance;
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
		KInstanceSelectionDialog(wxWindow* parent);
		virtual ~KInstanceSelectionDialog();

	public:
		KGameID GetNewGameID() const
		{
			return m_NewGameID;
		}
		wxString GetNewInstanceID() const
		{
			return m_NewInstance;
		}
		
		bool IsNewGameRootSet() const
		{
			return !m_NewGameRoot.IsEmpty();
		}
		wxString GetNewGameRoot() const
		{
			return m_NewGameRoot;
		}

	private:
		KGameInstance* GetSelectedTemplate() const;
		KGameInstance* GetSelectedInstance() const;

		void Configure();
		void LoadTemplatesList();
		void LoadInstancesList(const KGameInstance* instanceTemplate, const wxString& selectID);
		bool AskForGameFolder(const KGameInstance* instanceTemplate, const wxString& currentGamePath);

		void OnCreateShortcut(wxCommandEvent& event);
		void OnButton(wxNotifyEvent& event);
		void OnUpdateProfiles(wxNotifyEvent& event);
		void OnDisplayInstanceInfo(const KGameInstance* instance);
};
