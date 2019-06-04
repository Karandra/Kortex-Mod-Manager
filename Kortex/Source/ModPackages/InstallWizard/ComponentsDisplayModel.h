#pragma once
#include "stdafx.h"
#include "Utility/KDataViewTreeModel.h"
#include "Common.h"
#include "ComponentsDisplayModelNode.h"

namespace Kortex::InstallWizard
{
	class ComponentsDisplayModel: public KxDataViewTreeModelEx<KxDataViewModel>
	{
		private:
			const KPackageProjectComponents* m_ComponentsInfo = nullptr;
			const KPPCStep* m_Step = nullptr;
			ComponentsDisplayModelNode::Vector m_DataVector;
			KPPCEntry::RefVector m_CheckedEntries;

			const KPPCEntry* m_HotItem = nullptr;

		private:
			void OnInitControl() override;

			bool IsContainer(const KxDataViewItem& item) const override;
			KxDataViewItem GetParent(const KxDataViewItem& item) const override;
			void GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const override;
			bool GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attibutes, KxDataViewCellState cellState) const override;
		
			void GetValue(wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			bool SetValue(const wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column) override;
			bool IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const override;

		private:
			wxBitmap GetImageByTypeDescriptor(KPPCTypeDescriptor type) const;
			wxString GetMessageTypeDescriptor(KPPCTypeDescriptor type) const;
			KxDataViewBitmapTextToggleValue::ToggleType GetToggleType(KPPCSelectionMode mode) const;
			const wxString& GetSelectionModeString(const KPPCGroup& group) const;
			ComponentsDisplayModelNode::RefVector GetGroupNodes(const ComponentsDisplayModelNode* groupNode);
			bool NodeChanged(const ComponentsDisplayModelNode* node)
			{
				return ItemChanged(MakeItem(node));
			}
			bool IsEntryShouldBeChecked(const KPPCEntry* entry) const;
		
			void OnActivateItem(KxDataViewEvent& event);
			void OnHotTrackItem(KxDataViewEvent& event);

		public:
			ComponentsDisplayModel();

		public:
			size_t GetItemsCount() const;
			void SetDataVector();
			void SetDataVector(const KPackageProjectComponents* compInfo, const KPPCStep* step, const KPPCEntry::RefVector& checkedEntries);
			void RefreshItems() override;
			bool OnLeaveStep(KPPCEntry::RefVector& checkedEntries);

			const KPPCEntry* GetSelectedEntry() const;
			const KPPCEntry* GetHotTrackedEntry() const
			{
				return m_HotItem;
			}

			KxDataViewItem MakeItem(const ComponentsDisplayModelNode* node) const;
			KxDataViewItem MakeItem(const ComponentsDisplayModelNode& node) const
			{
				return MakeItem(&node);
			}
			ComponentsDisplayModelNode* GetNode(const KxDataViewItem& item) const;
			KPPCEntry::RefVector GetCheckedEntries() const;
	};
}
