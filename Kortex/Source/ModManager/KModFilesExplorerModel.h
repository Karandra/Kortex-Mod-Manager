#pragma once
#include "stdafx.h"
#include "KModFilesExplorerModelNode.h"
#include "KDataViewListModel.h"
#include "KImageProvider.h"

class KModFilesExplorerModel: public KDataViewModelBase
{
	private:
		const wxString m_ExplorerRoot;
		const KModEntry* m_ModEntry = NULL;
		KMFEModelNode::NodeVector m_Entries;

	private:
		virtual void OnInitControl() override;
		virtual bool IsContainer(const KxDataViewItem& item) const override;
		virtual bool HasContainerColumns(const KxDataViewItem& item) const override
		{
			return true;
		}
		virtual KxDataViewItem GetParent(const KxDataViewItem& item) const override;
		virtual void GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const override;
		
		virtual void GetEditorValue(wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column) const override;
		virtual void GetValue(wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column) const override;
		virtual bool SetValue(const wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column) override;
		virtual bool IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const override;

		void OnSelectItem(KxDataViewEvent& event);
		void OnActivateItem(KxDataViewEvent& event);
		void OnContextMenu(KxDataViewEvent& event);

	public:
		KModFilesExplorerModel(const wxString& sExplorerRoot, const KModEntry* modEntry = NULL);

	public:
		virtual void RefreshItems() override;

		KxDataViewItem MakeItem(const KMFEModelNode* node) const
		{
			return KxDataViewItem(reinterpret_cast<void*>(const_cast<KMFEModelNode*>(node)));
		}
		KxDataViewItem MakeItem(const KMFEModelNode& node) const
		{
			return MakeItem(&node);
		}
		KMFEModelNode* GetNode(const KxDataViewItem& item) const
		{
			return item.GetValuePtr<KMFEModelNode>();
		}

		bool HasModEntry() const
		{
			return m_ModEntry != NULL;
		}
};
