#pragma once
#include "stdafx.h"
#include "KDataViewListModel.h"
#include "KNotification.h"
#include "KBitmapSize.h"

class KNotificationPopupList: public KxDataViewVectorListModelEx<KNotification::Vector, KxDataViewListModelEx>
{
	private:
		KBitmapSize m_BitmapSize;
		KNotification::Vector& m_Data;

		KxDataViewColumn* m_BitmapColumn = NULL;
		KxDataViewColumn* m_MessageColumn = NULL;

	private:
		template<class EntryT, class VectorT> static EntryT* GetEntryFrom(VectorT& items, size_t index)
		{
			if (index < items.size())
			{
				return items[index].get();
			}
			return NULL;
		}

		virtual void OnInitControl() override;
		virtual void GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
		virtual bool GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attribute, KxDataViewCellState cellState) const override;
		virtual bool GetCellHeightByRow(size_t row, int& height) const override;

		void OnSelectItem(KxDataViewEvent& event);
		void OnActivateItem(KxDataViewEvent& event);

		wxString FormatToHTML(const KNotification& notification) const;

	public:
		KNotificationPopupList();

	public:
		void OnShowWindow(wxWindow* parent);

		const KNotification* GetDataEntry(size_t index) const
		{
			return GetEntryFrom<const KNotification>(m_Data, index);
		}
		KNotification* GetDataEntry(size_t index)
		{
			return GetEntryFrom<KNotification>(m_Data, index);
		}
		const KNotification* GetDataEntry(const KxDataViewItem& item) const
		{
			return GetEntryFrom<const KNotification>(m_Data, GetRow(item));
		}
		KNotification* GetDataEntry(const KxDataViewItem& item)
		{
			return GetEntryFrom<KNotification>(m_Data, GetRow(item));
		}
};
