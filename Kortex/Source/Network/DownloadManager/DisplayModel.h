#pragma once
#include "stdafx.h"
#include "Utility/KDataViewListModel.h"
#include "Network/IDownloadManager.h"
#include "DownloadItem.h"
class KxMenuEvent;

namespace Kortex::DownloadManager
{
	class DisplayModel: public KxDataViewVectorListModelEx<DownloadItem::Vector, KxDataViewListModelEx>
	{
		private:
			IDownloadManager* m_DownloadManager = nullptr;

		private:
			virtual void OnInitControl() override;

			virtual void GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			virtual void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			virtual bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
			virtual bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;
			virtual bool GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attribute, KxDataViewCellState cellState) const override;
			virtual bool HasDefaultCompare() const override
			{
				return true;
			}
			virtual bool CompareByRow(size_t row1, size_t row2, const KxDataViewColumn* column) const override;

			void OnActivateItem(KxDataViewEvent& event);
			void OnSelectItem(KxDataViewEvent& event);
			void OnContextMenu(KxDataViewEvent& event);

		private:
			wxBitmap GetStateBitmap(const DownloadItem& entry) const;
			void RemoveAll(bool installedOnly = false);
			void SetAllHidden(bool isHidden, bool installedOnly = false);
			void Install(DownloadItem& entry);

		public:
			DisplayModel();

		public:
			virtual size_t GetItemCount() const override
			{
				return HasDataVector() ? GetDataVector()->size() : 0;
			}

			DownloadItem* GetDataEntry(const size_t i)
			{
				if (i < GetItemCount())
				{
					return m_DownloadManager->GetDownloads()[i].get();
				}
				return nullptr;
			}
			const DownloadItem* GetDataEntry(const size_t i) const
			{
				if (i < GetItemCount())
				{
					return m_DownloadManager->GetDownloads()[i].get();
				}
				return nullptr;
			}
			KxDataViewItem FindItem(const DownloadItem& entry) const;
	};
}
