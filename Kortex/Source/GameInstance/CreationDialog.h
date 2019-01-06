#pragma once
#include "stdafx.h"
#include <KxFramework/KxSplitterWindow.h>
#include <KxFramework/KxComboBoxDialog.h>
class KxTextBox;
class KxCheckBox;
class KxComboBox;

namespace Kortex
{
	class IGameInstance;
}

namespace Kortex::GameInstance
{
	class CreationDialog: public KxComboBoxDialog
	{
		private:
			KxTextBox* m_NameInput = nullptr;
			KxComboBox* m_InstancesList = nullptr;
			KxCheckBox* m_CopyInstanceConfigCHK = nullptr;
			
			IGameInstance* m_InstanceTemplate = nullptr;
			wxString m_InstanceID;

		private:
			wxWindow* GetDialogFocusCtrl() const override;

		public:
			bool Create(wxWindow* parent, const GameID& gameID);
			CreationDialog() = default;
			CreationDialog(wxWindow* parent, const GameID& gameID)
			{
				Create(parent, gameID);
			}

		public:
			wxString GetInstanceID() const
			{
				return m_InstanceID;
			}

		private:
			const IGameInstance* GetSelectedInstance() const;

			void OnSelectInstance(wxCommandEvent& event);
			void OnButtonClick(wxNotifyEvent& event);
			bool OnOK(wxNotifyEvent& event);
	};
}
