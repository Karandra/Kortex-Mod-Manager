#pragma once
#include "stdafx.h"
#include "Utility/KImageProvider.h"
#include "PackageCreator/KPackageCreatorVectorModel.h"
#include "PackageCreator/KPackageCreatorIDTracker.h"
#include "PackageProject/KPackageProjectFileData.h"
class KOperationWithProgressBase;
class KPCFileDataFolderContentModel;
class KxDataViewSpinRenderer;

class KPCFileDataMainListModel: public KPackageCreatorVectorModel<KPPFFileEntryArray>, public KPackageCreatorIDTracker
{
	private:
		KPackageProjectFileData* m_FileData = nullptr;
		KPCFileDataFolderContentModel* m_ContentModel = nullptr;

		KxDataViewSpinEditor* m_PriorityRenderer = nullptr;

	private:
		virtual void OnInitControl() override;

		virtual void GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;

		void AddEverythingFromPath(const wxString& filePath, KPPFFolderEntry* fileEntry, KOperationWithProgressBase* context);
		bool DoTrackID(const wxString& trackedID, const wxString& newID, bool remove);

		void OnActivateItem(KxDataViewEvent& event);
		void OnSelectItem(KxDataViewEvent& event);
		void OnContextMenu(KxDataViewEvent& event);
		virtual void OnAllItemsMenuSelect(KxDataViewColumn* column) override;

		void OnAddFile();
		void OnAddFolder();
		void OnAddMultipleFolders();
		void OnReplaceFolderContent(const KxDataViewItem& item, KPPFFolderEntry* folderEntry);
		void OnRemoveElement(const KxDataViewItem& item);
		void OnClearList();
		virtual bool OnInsertItem(KxDataViewItem& currentItem, KxDataViewItem& droppedItem) override
		{
			OnInsertItemHelperUniquePtr(*GetDataVector(), currentItem, droppedItem);
			return true;
		}

	protected:
		virtual KPackageCreatorIDTracker* GetTracker() override
		{
			return this;
		}
		virtual bool TrackChangeID(const wxString& trackedID, const wxString& newID) override
		{
			return DoTrackID(trackedID, newID, false);
		}
		virtual bool TrackRemoveID(const wxString& trackedID) override
		{
			return DoTrackID(trackedID, wxEmptyString, true);
		}

	public:
		KPPFFileEntry* GetDataEntry(size_t index) const
		{
			if (index < GetItemCount())
			{
				return GetDataVector()->at(index).get();
			}
			return nullptr;
		}

		void SetProject(KPackageProject& projectData);
		void SetContentModel(KPCFileDataFolderContentModel* content)
		{
			m_ContentModel = content;
		}
};
