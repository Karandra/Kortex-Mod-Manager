#pragma once
#include "stdafx.h"
#include "KModManagerDispatcher.h"
#include "KDataViewListModel.h"
#include "KImageProvider.h"
class KModEntry;
class KFileTreeNode;

class KModFilesExplorerModel: public KxDataViewModelExBase<KxDataViewModel>
{
	private:
		const KModEntry& m_ModEntry;
		std::unordered_map<const KFileTreeNode*, KModManagerDispatcher::CollisionVector> m_Collisions;

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
		void OnExpandingItem(KxDataViewEvent& event);

		const KModManagerDispatcher::CollisionVector* GetCollisions(const KFileTreeNode& node) const;
		const KModManagerDispatcher::CollisionVector& GetOrUpdateCollisions(const KFileTreeNode& node);

	public:
		KModFilesExplorerModel(const KModEntry& modEntry)
			:m_ModEntry(modEntry)
		{
		}

	public:
		virtual void RefreshItems() override;

		KxDataViewItem MakeItem(const KFileTreeNode* node) const
		{
			return KxDataViewItem(node);
		}
		KxDataViewItem MakeItem(const KFileTreeNode& node) const
		{
			return MakeItem(&node);
		}
		KFileTreeNode* GetNode(const KxDataViewItem& item) const
		{
			return item.GetValuePtr<KFileTreeNode>();
		}
};
