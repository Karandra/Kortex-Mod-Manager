#pragma once
#include "stdafx.h"
#include "PackageCreator/KPackageCreatorVectorModel.h"
#include "PackageProject/KPackageProjectFileData.h"
class KxAuiToolBar;
class KxAuiToolBarItem;
class KxAuiToolBarEvent;

class KPCFileDataFolderContentModel: public KPackageCreatorVectorModel<KPPFFolderItemsArray>
{
	private:
		KPPFFolderEntry* m_Folder = nullptr;
		KPackageProjectFileData* m_FileData = nullptr;

	private:
		virtual void OnInitControl() override;

		virtual void GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;

	private:
		void OnActivateItem(KxDataViewEvent& event);
		void OnContextMenu(KxDataViewEvent& event);

		void OnAddMultipleFiles();
		void OnRemoveElement(const KxDataViewItem& item);
		void OnClearList();
		virtual bool OnInsertItem(KxDataViewItem& currentItem, KxDataViewItem& droppedItem) override
		{
			OnInsertItemHelperPrimitive(*GetDataVector(), currentItem, droppedItem);
			return true;
		}

	public:
		void SetProject(KPackageProject& projectData);
		void SetDataVector()
		{
			m_Folder = nullptr;
			KPackageCreatorVectorModel::SetDataVector(nullptr);
		}
		void SetDataVector(KPPFFolderEntry* folder)
		{
			m_Folder = folder;
			KPackageCreatorVectorModel::SetDataVector(&folder->GetFiles());
		}

		const KPPFFolderEntryItem* GetDataEntry(size_t index) const
		{
			if (index < GetItemCount())
			{
				return &GetDataVector()->at(index);
			}
			return nullptr;
		}
		KPPFFolderEntryItem* GetDataEntry(size_t index)
		{
			if (index < GetItemCount())
			{
				return &GetDataVector()->at(index);
			}
			return nullptr;
		}
};
