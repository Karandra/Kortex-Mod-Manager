#pragma once
#include "stdafx.h"
#include "SelectorDisplayModel.h"
#include <KxFramework/KxStdDialog.h>
class KxButton;

namespace Kortex::ModTagManager
{
	class SelectorDialog: public KxStdDialog, public SelectorDisplayModel
	{
		private:
			wxWindow* m_ViewPane = nullptr;
			KxButton* m_AddButton = nullptr;
			KxButton* m_RemoveButton = nullptr;
			KxButton* m_LoadDefaultTagsButton = nullptr;

		private:
			wxWindow* GetDialogMainCtrl() const override
			{
				return m_ViewPane;
			}
			void OnSelectItem(KxDataViewEvent& event);

			void OnAddTag(wxCommandEvent& event);
			void OnRemoveTag(wxCommandEvent& event);
			void OnLoadDefaultTags(wxCommandEvent& event);

		public:
			SelectorDialog(wxWindow* parent, const wxString& caption);
			virtual ~SelectorDialog();
	};
}
