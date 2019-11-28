#pragma once
#include "stdafx.h"
#include "PackageCreator/VectorModel.h"
#include "PackageProject/KPackageProjectRequirements.h"

namespace Kortex::PackageDesigner::PageRequirementsNS
{
	class EntriesListModel: public VectorModel<PackageProject::RequirementItem::Vector>
	{
		private:
			PackageProject::RequirementsSection* m_Requirements = nullptr;
			PackageProject::RequirementGroup* m_Group = nullptr;
	
			KxDataViewComboBoxEditor* m_TypeEditor = nullptr;
			KxDataViewComboBoxEditor* m_OperatorEditor = nullptr;
			KxDataViewComboBoxEditor* m_ObjectFunctionEditor = nullptr;
	
		private:
			void OnInitControl() override;
	
			void GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
			bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;
			bool GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const override;
	
			void OnActivateItem(KxDataViewEvent& event);
			void OnStartEditItem(KxDataViewEvent& event);
			void OnContextMenuItem(KxDataViewEvent& event);
			void OnAllItemsMenuSelect(KxDataViewColumn* column) override;
	
			void OnAddEntry();
			void OnRemoveEntry(const KxDataViewItem& item);
			void OnClearList();
			bool OnInsertItem(KxDataViewItem& currentItem, KxDataViewItem& droppedItem) override
			{
				OnInsertItemHelperUniquePtr(*GetDataVector(), currentItem, droppedItem);
				return true;
			}
	
		public:
			void SetProject(ModPackageProject& projectData);
			
			PackageProject::RequirementGroup* GetRequirementsGroup()
			{
				return m_Group;
			}
			void SetRequirementsGroup(PackageProject::RequirementGroup* group)
			{
				m_Group = group;
			}
	
			PackageProject::RequirementItem* GetDataEntry(size_t index) const
			{
				if (index < GetItemCount())
				{
					return GetDataVector()->at(index).get();
				}
				return nullptr;
			}
	};
}
