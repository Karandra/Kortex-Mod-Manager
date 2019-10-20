#pragma once
#include "stdafx.h"
#include "Utility/KDataViewListModel.h"
#include "Programs/IProgramItem.h"
#include "Programs/IProgramManager.h"
#include "Utility/KBitmapSize.h"

namespace Kortex::ProgramManager
{
	class DisplayModelDND: public KxDataViewModelExDragDropData
	{
		private:
			KxDataViewItem m_Item;

		public:
			DisplayModelDND(const KxDataViewItem& item)
				:m_Item(item)
			{
			}

		public:
			KxDataViewItem GetItem() const
			{
				return m_Item;
			}
	};
}

namespace Kortex::ProgramManager
{
	class DisplayModel:
		public KxDataViewVectorListModelEx<IProgramItem::Vector, KxDataViewListModelEx>,
		public KxDataViewModelExDragDropEnabled<DisplayModelDND>
	{
		private:
			KBitmapSize m_BitmapSize;
			bool m_ShowExpandedValues = false;

		private:
			void OnInitControl() override;
		
			void GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
			bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;
			bool GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const override;

			void OnActivateItem(KxDataViewEvent& event);
			void OnSelectItem(KxDataViewEvent& event);
			void OnContextMenu(KxDataViewEvent& event);

			KxDataViewCtrl* GetViewCtrl() const override
			{
				return GetView();
			}
			bool OnDragItems(KxDataViewEventDND& event) override;
			bool OnDropItems(KxDataViewEventDND& event) override;
			bool CanDragDropNow() const;

			bool AddProgram();
			void RemoveProgram(IProgramItem& item);
			wxString AskSelectIcon(const IProgramItem& item) const;

			bool SaveLoadExpandedValues(bool save, bool value = false) const;;

		public:
			DisplayModel();

		public:
			void RefreshItems() override;
			IProgramItem* GetDataEntry(size_t i) const
			{
				if (i < GetItemCount())
				{
					return (*GetDataVector())[i].get();
				}
				return nullptr;
			}
	};
}
