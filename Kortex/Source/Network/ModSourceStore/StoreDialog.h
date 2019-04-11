#pragma once
#include "stdafx.h"
#include "StoreDisplayModel.h"
#include <KxFramework/KxStdDialog.h>
#include <KxFramework/KxMenu.h>
class KxButton;

namespace Kortex::ModSource
{
	class StoreDialog: public KxStdDialog, public StoreDisplayModel
	{
		private:
			wxWindow* m_ViewPane = nullptr;
			KxButton* m_AddButton = nullptr;
			KxButton* m_RemoveButton = nullptr;
			KxMenu m_AddButtonMenu;

		private:
			wxWindow* GetDialogMainCtrl() const override
			{
				return m_ViewPane;
			}
			void CreateAddMenu();
			void OnAddItem(ModSourceItem& node);

			void OnSelectItem(KxDataViewEvent& event);
			void OnAddProvider(wxCommandEvent& event);
			void OnRemoveProvider(wxCommandEvent& event);
			void OnCloseDialog(wxCloseEvent& event);

		public:
			StoreDialog(wxWindow* parent, ModSourceStore& store);
			virtual ~StoreDialog();
	};
}
