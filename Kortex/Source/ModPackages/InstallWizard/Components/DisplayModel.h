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
			const PackageProject::KPackageProjectComponents* m_ComponentsInfo = nullptr;
			const PackageProject::KPPCStep* m_Step = nullptr;
			DisplayModelNode::Vector m_DataVector;
			PackageProject::KPPCEntry::RefVector m_CheckedEntries;

			const PackageProject::KPPCEntry* m_HotItem = nullptr;

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
			wxBitmap GetImageByTypeDescriptor(PackageProject::KPPCTypeDescriptor type) const;
			wxString GetMessageTypeDescriptor(PackageProject::KPPCTypeDescriptor type) const;
			KxDataViewBitmapTextToggleValue::ToggleType GetToggleType(PackageProject::KPPCSelectionMode mode) const;
			const wxString& GetSelectionModeString(const PackageProject::KPPCGroup& group) const;
			DisplayModelNode::RefVector GetGroupNodes(const DisplayModelNode* groupNode);
			bool NodeChanged(const DisplayModelNode* node)
			{
				return ItemChanged(MakeItem(node));
			}
			bool IsEntryShouldBeChecked(const PackageProject::KPPCEntry* entry) const;
		
			void OnActivateItem(KxDataViewEvent& event);
			void OnHotTrackItem(KxDataViewEvent& event);

		public:
			DisplayModel();

		public:
			size_t GetItemsCount() const;
			void SetDataVector();
			void SetDataVector(const PackageProject::KPackageProjectComponents* compInfo, const PackageProject::KPPCStep* step, const PackageProject::KPPCEntry::RefVector& checkedEntries);
			void RefreshItems() override;
			bool OnLeaveStep(PackageProject::KPPCEntry::RefVector& checkedEntries);

			const PackageProject::KPPCEntry* GetSelectedEntry() const;
			const PackageProject::KPPCEntry* GetHotTrackedEntry() const
			{
				return m_HotItem;
			}

			KxDataViewItem MakeItem(const DisplayModelNode* node) const;
			KxDataViewItem MakeItem(const DisplayModelNode& node) const
			{
				return MakeItem(&node);
			}
			DisplayModelNode* GetNode(const KxDataViewItem& item) const;
			PackageProject::KPPCEntry::RefVector GetCheckedEntries() const;
	};
}
