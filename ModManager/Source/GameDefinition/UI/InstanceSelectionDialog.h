#pragma once
#include "Framework.hpp"
#include "FrameworkUI.hpp"
#include <kxf/UI/Dialogs/StdDialog.h>
#include <kxf/UI/Controls/DataView.h>

namespace Kortex
{
	class IGameInstance;
}

namespace Kortex::GameDefinition::UI
{
	class InstanceSelectionDialog: public kxf::UI::StdDialog
	{
		private:
			class DataModel;
			class DataItem;

		private:
			DataView::View* m_View = nullptr;
			kxf::UI::Button* m_ButtonOK = nullptr;

			IGameInstance* m_SelectedInstance = nullptr;

		private:
			void OnSelectItem(DataView::ItemEvent& event);
			
		public:
			InstanceSelectionDialog(wxWindow* parent);

		public:
			// kxf::UI::StdDialog
			wxWindow* GetDialogMainCtrl() const override
			{
				return m_View;
			}

		public:
			IGameInstance* GetSelectedInstance() const
			{
				return m_SelectedInstance;
			}
	};
}
