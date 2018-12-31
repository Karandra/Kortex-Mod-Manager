#pragma once
#include "stdafx.h"
#include "Utility/KDataViewListModel.h"
#include "BaseNotification.h"
#include "Utility/KBitmapSize.h"

namespace Kortex::Notification
{
	class DisplayModel: public KxDataViewVectorListModelEx<INotification::Vector, KxDataViewListModelEx>
	{
		private:
			KBitmapSize m_BitmapSize;
			INotification::Vector& m_Data;

			KxDataViewColumn* m_BitmapColumn = nullptr;
			KxDataViewColumn* m_MessageColumn = nullptr;

		private:
			template<class EntryT, class VectorT> static EntryT* GetEntryFrom(VectorT& items, size_t index)
			{
				if (index < items.size())
				{
					return items[index].get();
				}
				return nullptr;
			}

			virtual void OnInitControl() override;
			virtual void GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
			virtual bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
			virtual bool GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attribute, KxDataViewCellState cellState) const override;
			virtual bool GetCellHeightByRow(size_t row, int& height) const override;

			void OnSelectItem(KxDataViewEvent& event);
			void OnActivateItem(KxDataViewEvent& event);

			wxString FormatToHTML(const INotification& notification) const;

		public:
			DisplayModel();

		public:
			void OnShowWindow(wxWindow* parent);

			const INotification* GetDataEntry(size_t index) const
			{
				return GetEntryFrom<const INotification>(m_Data, index);
			}
			INotification* GetDataEntry(size_t index)
			{
				return GetEntryFrom<INotification>(m_Data, index);
			}
			const INotification* GetDataEntry(const KxDataViewItem& item) const
			{
				return GetEntryFrom<const INotification>(m_Data, GetRow(item));
			}
			INotification* GetDataEntry(const KxDataViewItem& item)
			{
				return GetEntryFrom<INotification>(m_Data, GetRow(item));
			}
	};
}
