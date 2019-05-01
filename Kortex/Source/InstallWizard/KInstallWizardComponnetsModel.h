#pragma once
#include "stdafx.h"
#include "Utility/KDataViewTreeModel.h"
#include "KInstallWizardDefs.h"
#include "KInstallWizardComponnetsModelNode.h"

class KInstallWizardComponnetsModel: public KDataViewTreeModel
{
	private:
		const KPackageProjectComponents* m_ComponentsInfo = nullptr;
		const KPPCStep* m_Step = nullptr;
		KIWCNodesVector m_DataVector;
		KPPCEntry::RefVector m_CheckedEntries;

		const KPPCEntry* m_HotItem = nullptr;

	private:
		virtual void OnInitControl() override;

		virtual bool IsContainer(const KxDataViewItem& item) const override;
		virtual KxDataViewItem GetParent(const KxDataViewItem& item) const override;
		virtual void GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const override;
		virtual bool GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attibutes, KxDataViewCellState cellState) const override;
		
		virtual void GetValue(wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column) const override;
		virtual bool SetValue(const wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column) override;
		virtual bool IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const override;

	private:
		wxBitmap GetImageByTypeDescriptor(KPPCTypeDescriptor type) const;
		wxString GetMessageTypeDescriptor(KPPCTypeDescriptor type) const;
		KxDataViewBitmapTextToggleValue::ToggleType GetToggleType(KPPCSelectionMode mode) const;
		const wxString& GetSelectionModeString(const KPPCGroup& group) const;
		KIWCNodesRefVector GetGroupNodes(const KIWCModelNode* groupNode);
		bool NodeChanged(const KIWCModelNode* node)
		{
			return ItemChanged(MakeItem(node));
		}
		bool IsEntryShouldBeChecked(const KPPCEntry* entry) const;
		
		void OnActivateItem(KxDataViewEvent& event);
		void OnHotTrackItem(KxDataViewEvent& event);

	public:
		KInstallWizardComponnetsModel();

	public:
		size_t GetItemsCount() const;
		void SetDataVector();
		void SetDataVector(const KPackageProjectComponents* compInfo, const KPPCStep* step, const KPPCEntry::RefVector& checkedEntries);
		virtual void RefreshItems() override;
		virtual bool OnLeaveStep(KPPCEntry::RefVector& checkedEntries);

		const KPPCEntry* GetSelectedEntry() const;
		const KPPCEntry* GetHotTrackedEntry() const
		{
			return m_HotItem;
		}

		KxDataViewItem MakeItem(const KIWCModelNode* node) const;
		KxDataViewItem MakeItem(const KIWCModelNode& node) const
		{
			return MakeItem(&node);
		}
		KIWCModelNode* GetNode(const KxDataViewItem& item) const;
		KPPCEntry::RefVector GetCheckedEntries() const;
};
