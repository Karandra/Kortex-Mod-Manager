#pragma once
#include "stdafx.h"
#include "PackageCreator/KPackageCreatorVectorModel.h"
#include "PackageProject/KPackageProjectInterface.h"
#include "PackageCreator/KPackageCreatorIDTracker.h"
#include "Utility/KBitmapSize.h"

namespace Kortex::PackageDesigner
{
	class KPCIImagesListModel: public KPackageCreatorVectorModel<KPPIImageEntryArray>, public KPackageCreatorIDTracker
	{
		public:
			static KBitmapSize GetThumbnailSize();
			static void LoadBitmap(KPPIImageEntry* entry);
	
		private:
			KPackageProjectInterface* m_Interface = nullptr;
	
		private:
			void OnInitControl() override;
	
			void GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
			bool SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column) override;
	
			void OnActivateItem(KxDataViewEvent& event);
			void OnContextMenu(KxDataViewEvent& event);
			void OnCacheHint(KxDataViewEvent& event);
			void OnAllItemsMenuSelect(KxDataViewColumn* column) override;
	
			void OnImportFiles();
			void OnAddMultipleItems();
			void OnRemoveEntry(const KxDataViewItem& item);
			void OnClearList();
			bool OnInsertItem(KxDataViewItem& currentItem, KxDataViewItem& droppedItem) override
			{
				OnInsertItemHelperPrimitive(*GetDataVector(), currentItem, droppedItem);
				return true;
			}
	
			bool DoTrackImagePath(const wxString& trackedID, const wxString& newID, bool remove) const;
	
		protected:
			KPackageCreatorIDTracker* GetTracker() override
			{
				return this;
			}
			bool TrackChangeID(const wxString& trackedID, const wxString& newID) override
			{
				return DoTrackImagePath(trackedID, newID, false);
			}
			bool TrackRemoveID(const wxString& trackedID) override
			{
				return DoTrackImagePath(trackedID, wxEmptyString, true);
			}
	
		public:
			const KPPIImageEntry* GetDataEntry(size_t index) const
			{
				if (index < GetItemCount())
				{
					return &GetDataVector()->at(index);
				}
				return nullptr;
			}
			KPPIImageEntry* GetDataEntry(size_t index)
			{
				if (index < GetItemCount())
				{
					return &GetDataVector()->at(index);
				}
				return nullptr;
			}
	
			const KPPIImageEntry* GetDataEntry(const KxDataViewItem& item) const
			{
				return GetDataEntry(GetRow(item));
			}
	
			void SetProject(KPackageProject& projectData);
	};
}
