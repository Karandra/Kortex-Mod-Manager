#pragma once
#include "stdafx.h"
#include "PackageCreator/VectorModel.h"
#include "PackageProject/FileDataSection.h"
class KxAuiToolBar;
class KxAuiToolBarItem;
class KxAuiToolBarEvent;

namespace Kortex::PackageDesigner::PageFileDataNS
{
	class ContentModel: public VectorModel<std::vector<PackageProject::FileItem>>
	{
		private:
			PackageProject::FileDataSection* m_FileData = nullptr;
			PackageProject::FolderItem* m_Folder = nullptr;
	
		private:
			void OnInitControl() override;
	
			void GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
	
		private:
			void OnActivateItem(KxDataViewEvent& event);
			void OnContextMenu(KxDataViewEvent& event);
	
			void OnAddMultipleFiles();
			void OnRemoveElement(const KxDataViewItem& item);
			void OnClearList();
			bool OnInsertItem(KxDataViewItem& currentItem, KxDataViewItem& droppedItem) override
			{
				OnInsertItemHelperPrimitive(*GetDataVector(), currentItem, droppedItem);
				return true;
			}
	
		public:
			void SetProject(ModPackageProject& projectData);
			void SetDataVector()
			{
				m_Folder = nullptr;
				VectorModel::SetDataVector(nullptr);
			}
			void SetDataVector(PackageProject::FolderItem* folder)
			{
				m_Folder = folder;
				VectorModel::SetDataVector(&folder->GetFiles());
			}
	
			const PackageProject::FileItem* GetDataEntry(size_t index) const
			{
				if (index < GetItemCount())
				{
					return &GetDataVector()->at(index);
				}
				return nullptr;
			}
			PackageProject::FileItem* GetDataEntry(size_t index)
			{
				if (index < GetItemCount())
				{
					return &GetDataVector()->at(index);
				}
				return nullptr;
			}
	};
}
