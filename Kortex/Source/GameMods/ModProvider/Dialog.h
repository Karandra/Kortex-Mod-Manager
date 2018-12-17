#pragma once
#include "stdafx.h"
#include "DisplayModel.h"
#include <KxFramework/KxStdDialog.h>
class KxButton;

namespace Kortex::ModProvider
{
	class Dialog: public KxStdDialog, public DisplayModel
	{
		private:
			wxWindow* m_ViewPane = nullptr;
			KxButton* m_AddButton = nullptr;
			KxButton* m_RemoveButton = nullptr;

		private:
			virtual wxWindow* GetDialogMainCtrl() const override
			{
				return m_ViewPane;
			}
			void OnSelectItem(KxDataViewEvent& event);
			void OnAddProvider(wxCommandEvent& event);
			void OnRemoveProvider(wxCommandEvent& event);

		public:
			Dialog(wxWindow* parent, Store& providerStore);
			virtual ~Dialog();
	};
}
