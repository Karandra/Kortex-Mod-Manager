#pragma once
#include "stdafx.h"
#include "PackageCreator/KPackageCreatorVectorModel.h"
#include "PackageProject/KPackageProjectRequirements.h"

class KPCREntriesListModel: public KPackageCreatorVectorModel<KPPRRequirementEntry::Vector>
{
	private:
		KPackageProjectRequirements* m_Requirements = NULL;
		KPPRRequirementsGroup* m_Group = NULL;

		KxDataViewComboBoxEditor* m_TypeEditor = NULL;
		KxDataViewComboBoxEditor* m_OperatorEditor = NULL;
		KxDataViewComboBoxEditor* m_ObjectFunctionEditor = NULL;

	private:
		virtual void OnInitControl() override;

		virtual void GetEditorValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
		virtual bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
		virtual bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;
		virtual bool GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const override;

		void OnActivateItem(KxDataViewEvent& event);
		void OnStartEditItem(KxDataViewEvent& event);
		void OnContextMenuItem(KxDataViewEvent& event);
		virtual void OnAllItemsMenuSelect(KxDataViewColumn* column) override;

		void OnAddEntry();
		void OnRemoveEntry(const KxDataViewItem& item);
		void OnClearList();
		virtual bool OnInsertItem(KxDataViewItem& currentItem, KxDataViewItem& droppedItem) override
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
			return NULL;
		}
};
