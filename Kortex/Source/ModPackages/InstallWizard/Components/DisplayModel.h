#pragma once
#include "stdafx.h"
#include "../Common.h"
#include "DisplayModelNode.h"
#include <KxFramework/KxDataView.h>
#include "KxFramework/KxDataViewTreeModelEx.h"

namespace Kortex::InstallWizard::ComponentsPageNS
{
	class DisplayModel: public KxDataViewTreeModelEx<KxDataViewModel>
	{
		private:
			const PackageProject::ComponentsSection* m_ComponentsInfo = nullptr;
			const PackageProject::ComponentStep* m_Step = nullptr;
			DisplayModelNode::Vector m_DataVector;
			PackageProject::ComponentItem::RefVector m_CheckedEntries;

			const PackageProject::ComponentItem* m_HotItem = nullptr;

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
			wxBitmap GetImageByTypeDescriptor(PackageProject::TypeDescriptor type) const;
			wxString GetMessageTypeDescriptor(PackageProject::TypeDescriptor type) const;
			KxDataViewBitmapTextToggleValue::ToggleType GetToggleType(PackageProject::SelectionMode mode) const;
			const wxString& GetSelectionModeString(const PackageProject::ComponentGroup& group) const;
			DisplayModelNode::RefVector GetGroupNodes(const DisplayModelNode* groupNode);
			bool NodeChanged(const DisplayModelNode* node)
			{
				return ItemChanged(MakeItem(node));
			}
			bool IsEntryShouldBeChecked(const PackageProject::ComponentItem* entry) const;
		
			void OnActivateItem(KxDataViewEvent& event);
			void OnHotTrackItem(KxDataViewEvent& event);

		public:
			DisplayModel();

		public:
			size_t GetItemsCount() const;
			void SetDataVector();
			void SetDataVector(const PackageProject::ComponentsSection* compInfo, const PackageProject::ComponentStep* step, const PackageProject::ComponentItem::RefVector& checkedEntries);
			void RefreshItems() override;
			bool OnLeaveStep(PackageProject::ComponentItem::RefVector& checkedEntries);

			const PackageProject::ComponentItem* GetSelectedEntry() const;
			const PackageProject::ComponentItem* GetHotTrackedEntry() const
			{
				return m_HotItem;
			}

			KxDataViewItem MakeItem(const DisplayModelNode* node) const;
			KxDataViewItem MakeItem(const DisplayModelNode& node) const
			{
				return MakeItem(&node);
			}
			DisplayModelNode* GetNode(const KxDataViewItem& item) const;
			PackageProject::ComponentItem::RefVector GetCheckedEntries() const;
	};
}
