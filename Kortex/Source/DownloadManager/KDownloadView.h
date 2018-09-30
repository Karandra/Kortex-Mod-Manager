#pragma once
#include "stdafx.h"
#include "KDataViewListModel.h"
#include "KDownloadManager.h"
class KxMenuEvent;

class KDownloadView: public KDataViewVectorListModel<KDownloadEntry::Vector, KDataViewListModel>
{
	private:
		virtual void OnInitControl() override;

		virtual void GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
		virtual bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;
		virtual bool HasDefaultCompare() const override
		{
			return true;
		}
		virtual bool CompareByRow(size_t row1, size_t row2, const KxDataViewColumn* column) const override;

		void OnActivateItem(KxDataViewEvent& event);
		void OnSelectItem(KxDataViewEvent& event);
		void OnContextMenu(KxDataViewEvent& event);
		void OnContextMenuSelected(KxMenuEvent& event, KDownloadEntry* entry = NULL);

	private:
		wxBitmap GetStateBitmap(const KDownloadEntry& entry) const;
		void RemoveAll(bool installedOnly = false);
		void SetAllHidden(bool isHidden, bool installedOnly = false);
		void Install(KDownloadEntry& entry);

	public:
		KDownloadView();

	public:
		virtual size_t GetItemCount() const override
		{
			return HasDataVector() ? GetDataVector()->size() : 0;
		}

		KDownloadEntry* GetDataEntry(const size_t i)
		{
			if (i < GetItemCount())
			{
				return KDownloadManager::GetInstance()->GetDownloads()[i].get();
			}
			return NULL;
		}
		const KDownloadEntry* GetDataEntry(const size_t i) const
		{
			if (i < GetItemCount())
			{
				return KDownloadManager::GetInstance()->GetDownloads()[i].get();
			}
			return NULL;
		}
		KxDataViewItem FindItem(const KDownloadEntry& entry) const
		{
			auto it = KDownloadManager::GetInstance()->GetDownloadIterator(entry);
			if (it != KDownloadManager::GetInstance()->GetDownloads().end())
			{
				return GetItem(std::distance(GetDataVector()->begin(), it));
			}
			return KxDataViewItem();
		}
};
