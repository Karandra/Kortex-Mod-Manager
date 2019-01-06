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
			KxBitmapComboBox* m_GameFilter = nullptr;
			KxImageList* m_GameFilterImageList = nullptr;
			KxListBox* m_InstancesList = nullptr;

			KxPanel* m_RightPane = nullptr;
			wxBoxSizer* m_RightSizer = nullptr;
			KxTextBox* m_TextBox = nullptr;

			KxButton* m_OK = nullptr;
			KxButton* m_Create = nullptr;
			KxButton* m_Remove = nullptr;
			KxButton* m_CreateShortcut = nullptr;

		private:
			wxString m_SelectedGameRoot;
			IGameInstance* m_SelectedInstance = nullptr;

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
			IGameInstance* GetSelectedInstance() const
			{
				return m_SelectedInstance;
			}

			bool IsGameRootSelected() const
			{
				return !m_SelectedGameRoot.IsEmpty();
			}
			wxString GetSelectedGameRoot() const
			{
				return m_SelectedGameRoot;
			}

		private:
			GameID GetCurrentFilter() const;
			IGameInstance* GetCurrentInstance() const;

			void Configure();
			void LoadGameFilter(const GameID& gameID = GameIDs::NullGameID);
			void LoadInstancesList(const GameID& gameID = GameIDs::NullGameID, IGameInstance* selectInstance = nullptr);
			bool AskForGameFolder(const IGameInstance* instance, const wxString& currentGamePath);

			void OnFilterSelected(const GameID& gameID = GameIDs::NullGameID);
			void OnInstanceSelected(IGameInstance* instance = nullptr);

			void OnCreateShortcut(wxCommandEvent& event);
			void OnButton(wxNotifyEvent& event);
			void OnUpdateProfiles(wxNotifyEvent& event);
			void OnDisplayInstanceInfo(const IGameInstance* instance);
	};
}
