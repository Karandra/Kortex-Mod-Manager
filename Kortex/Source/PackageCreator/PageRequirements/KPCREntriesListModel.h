#pragma once
#include "stdafx.h"
#include "PackageCreator/KPackageCreatorVectorModel.h"
#include "PackageProject/KPackageProjectRequirements.h"

namespace Kortex::PackageDesigner
{
	class KPCREntriesListModel: public KPackageCreatorVectorModel<KPPRRequirementEntry::Vector>
	{
		private:
			KPackageProjectRequirements* m_Requirements = nullptr;
			KPPRRequirementsGroup* m_Group = nullptr;
	
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
			void SetProject(KPackageProject& projectData);
			
			KPPRRequirementsGroup* GetRequirementsGroup()
			{
				return m_Group;
			}
			void SetRequirementsGroup(KPPRRequirementsGroup* group)
			{
				m_Group = group;
			}
	
			KPPRRequirementEntry* GetDataEntry(size_t index) const
			{
				if (index < GetItemCount())
				{
					return GetDataVector()->at(index).get();
				}
				return nullptr;
			}
	};
}
