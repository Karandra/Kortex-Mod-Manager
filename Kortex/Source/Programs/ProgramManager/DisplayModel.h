#pragma once
#include "stdafx.h"
#include "KDataViewListModel.h"
#include "Programs/IProgramEntry.h"
#include "Programs/IProgramManager.h"
#include "KBitmapSize.h"

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
		public KxDataViewVectorListModelEx<IProgramEntry::Vector, KxDataViewListModelEx>,
		public KxDataViewModelExDragDropEnabled<DisplayModelDND>
	{
		private:
			KBitmapSize m_BitmapSize;
			bool m_ShowExpandedValues = false;

		private:
			virtual void OnInitControl() override;
		
			virtual void GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			virtual void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			virtual bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
			virtual bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;
			virtual bool GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const override;

			void OnActivateItem(KxDataViewEvent& event);
			void OnSelectItem(KxDataViewEvent& event);
			void OnContextMenu(KxDataViewEvent& event);

			virtual KxDataViewCtrl* GetViewCtrl() const override
			{
				return GetView();
			}
			virtual bool OnDragItems(KxDataViewEventDND& event) override;
			virtual bool OnDropItems(KxDataViewEventDND& event) override;
			bool CanDragDropNow() const;

			wxString AskSelectExecutable(const IProgramEntry* entry = nullptr) const;
			wxString AskSelectIcon(const IProgramEntry& entry) const;
			bool AddProgram();
			void RemoveProgram(IProgramEntry* entry);

			bool SaveLoadExpandedValues(bool save, bool value = false) const;;

		public:
			DisplayModel();

		public:
			virtual void RefreshItems() override;
			IProgramEntry* GetDataEntry(size_t i) const
			{
				if (i < GetItemCount())
				{
					return (*GetDataVector())[i].get();
				}
				return nullptr;
			}
	};
}
