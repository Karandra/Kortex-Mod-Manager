#pragma once
#include "stdafx.h"
#include "GameInstance/GameID.h"
#include <KxFramework/KxComboBoxDialog.h>
class KxPanel;
class KxButton;
class KxTextBox;
class KxListBox;
class KxBitmapComboBox;
class KxSplitterWindow;

namespace Kortex
{
	class IGameInstance;

	class KInstanceSelectionDialog: public KxComboBoxDialog
	{
		private:
			KxSplitterWindow* m_Splitter = nullptr;

			KxPanel* m_LeftPane = nullptr;
			wxBoxSizer* m_LeftSizer = nullptr;
			KxBitmapComboBox* m_TemplatesList = nullptr;
			KxListBox* m_InstancesList = nullptr;

			KxPanel* m_RightPane = nullptr;
			wxBoxSizer* m_RightSizer = nullptr;
			KxTextBox* m_TextBox = nullptr;

			KxButton* m_OK = nullptr;
			KxButton* m_Create = nullptr;
			KxButton* m_Remove = nullptr;
			KxButton* m_CreateShortcut = nullptr;

		private:
			GameID m_NewGameID;
			wxString m_NewInstanceID;
			wxString m_NewGameRoot;
			IGameInstance* m_NewInstance = nullptr;

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
			GameID GetNewGameID() const
			{
				return m_NewGameID;
			}
			wxString GetNewInstanceID() const
			{
				return m_NewInstanceID;
			}
			IGameInstance* GetNewInstance() const
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
			IGameInstance* GetSelectedTemplate() const;
			IGameInstance* GetSelectedInstance() const;

			void Configure();
			void LoadTemplatesList();
			void LoadInstancesList(const IGameInstance* instanceTemplate, const wxString& selectID);
			bool AskForGameFolder(const IGameInstance* instanceTemplate, const wxString& currentGamePath);

			void OnCreateShortcut(wxCommandEvent& event);
			void OnButton(wxNotifyEvent& event);
			void OnUpdateProfiles(wxNotifyEvent& event);
			void OnDisplayInstanceInfo(const IGameInstance* instance);
	};
}
