#pragma once
#include "stdafx.h"
#include "../Common.h"
#include "DisplayModelNode.h"
#include <KxFramework/KxDataView.h>
#include "Utility/KDataViewTreeModel.h"

namespace Kortex::InstallWizard::ComponentsPageNS
{
	class DisplayModel: public KxDataViewTreeModelEx<KxDataViewModel>
	{
		private:
			const PackageDesigner::KPackageProjectComponents* m_ComponentsInfo = nullptr;
			const PackageDesigner::KPPCStep* m_Step = nullptr;
			DisplayModelNode::Vector m_DataVector;
			PackageDesigner::KPPCEntry::RefVector m_CheckedEntries;

			const PackageDesigner::KPPCEntry* m_HotItem = nullptr;

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
			wxBitmap GetImageByTypeDescriptor(PackageDesigner::KPPCTypeDescriptor type) const;
			wxString GetMessageTypeDescriptor(PackageDesigner::KPPCTypeDescriptor type) const;
			KxDataViewBitmapTextToggleValue::ToggleType GetToggleType(PackageDesigner::KPPCSelectionMode mode) const;
			const wxString& GetSelectionModeString(const PackageDesigner::KPPCGroup& group) const;
			DisplayModelNode::RefVector GetGroupNodes(const DisplayModelNode* groupNode);
			bool NodeChanged(const DisplayModelNode* node)
			{
				return ItemChanged(MakeItem(node));
			}
			bool IsEntryShouldBeChecked(const PackageDesigner::KPPCEntry* entry) const;
		
			void OnActivateItem(KxDataViewEvent& event);
			void OnHotTrackItem(KxDataViewEvent& event);

		public:
			DisplayModel();

		public:
			size_t GetItemsCount() const;
			void SetDataVector();
			void SetDataVector(const PackageDesigner::KPackageProjectComponents* compInfo, const PackageDesigner::KPPCStep* step, const PackageDesigner::KPPCEntry::RefVector& checkedEntries);
			void RefreshItems() override;
			bool OnLeaveStep(PackageDesigner::KPPCEntry::RefVector& checkedEntries);

			const PackageDesigner::KPPCEntry* GetSelectedEntry() const;
			const PackageDesigner::KPPCEntry* GetHotTrackedEntry() const
			{
				return m_HotItem;
			}

			KxDataViewItem MakeItem(const DisplayModelNode* node) const;
			KxDataViewItem MakeItem(const DisplayModelNode& node) const
			{
				return MakeItem(&node);
			}
			DisplayModelNode* GetNode(const KxDataViewItem& item) const;
			PackageDesigner::KPPCEntry::RefVector GetCheckedEntries() const;
	};
}
