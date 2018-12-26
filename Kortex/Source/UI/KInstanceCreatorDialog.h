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

	class KInstanceCreatorDialog: public KxComboBoxDialog
	{
		private:
			IGameInstance* m_InstanceTemplate = nullptr;
			KxTextBox* m_NameInput = nullptr;
			KxComboBox* m_InstancesList = nullptr;
		
			KxCheckBox* m_CopyInstanceConfigCHK = nullptr;
			KxCheckBox* m_CopyModTagsCHK = nullptr;
		
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
			KInstanceCreatorDialog(wxWindow* parent, IGameInstance* instanceTemplate);
			virtual ~KInstanceCreatorDialog();

		public:
			const wxString& GetConfigID() const
			{
				return m_Name;
			}

		private:
			const IGameInstance* GetSelectedInstance() const;

			void OnSelectInstance(wxCommandEvent& event);
			void OnButtonClick(wxNotifyEvent& event);
			bool OnOK(wxNotifyEvent& event);
	};
}
