#pragma once
#include "stdafx.h"
#include "KDataViewListModel.h"
#include "KProgramManager.h"
#include "KBitmapSize.h"

class KProgramManagerModelDND: public KxDataViewModelExDragDropData
{
	private:
		KxDataViewItem m_Item;

	public:
		KProgramManagerModelDND(const KxDataViewItem& item)
			:m_Item(item)
		{
		}

	public:
		KxDataViewItem GetItem() const
		{
			return m_Item;
		}
};

class KProgramManagerModel:
	public KxDataViewVectorListModelEx<KProgramEntry::Vector, KxDataViewListModelEx>,
	public KxDataViewModelExDragDropEnabled<KProgramManagerModelDND>
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

		KProgramEntry::Vector& GetProgramsList() const
		{
			return KProgramManager::GetInstance()->GetProgramList();
		}
		KProgramEntry* GetDataEntry(size_t i)
		{
			if (i < GetItemCount())
			{
				return &GetDataVector()->at(i);
			}
			return NULL;
		}
		
		wxString AskSelectExecutable(const KProgramEntry* entry = NULL) const;
		wxString AskSelectIcon(const KProgramEntry& entry) const;
		bool AddProgram();
		void RemoveProgram(KProgramEntry* entry);

		bool SaveLoadExpandedValues(bool save, bool value = false) const;;

	public:
		KProgramManagerModel();

	public:
		virtual size_t GetItemCount() const override
		{
			return GetProgramsList().size();
		}
		virtual void RefreshItems() override;

		const KProgramEntry* GetDataEntry(size_t i) const
		{
			if (i < GetItemCount())
			{
				return &GetDataVector()->at(i);
			}
			return NULL;
		}
		const KProgramEntry* GetDataEntry(const KxDataViewItem& item) const
		{
			return GetDataEntry(GetRow(item));
		}
};
