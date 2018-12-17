#pragma once
#include "stdafx.h"
#include "DisplayModel.h"
#include <KxFramework/KxStdDialog.h>
class KxButton;

namespace Kortex::ModStatistics
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

		public:
			Dialog(wxWindow* parent);
			virtual ~Dialog();
	};
}
