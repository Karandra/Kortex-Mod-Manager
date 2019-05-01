#pragma once
#include "stdafx.h"
#include <Kortex/ModManager.hpp>
#include "Utility/KDataViewListModel.h"

namespace Kortex
{
	class IGameMod;
	class FileTreeNode;
}

namespace Kortex::VirtualGameFolder
{
	class DisplayModel: public KxDataViewModelExBase<KxDataViewModel>
	{
		private:
			const FileTreeNode::Vector* m_TreeItems = nullptr;
			FileTreeNode::Vector m_FoundItems;
			mutable std::unordered_map<const FileTreeNode*, wxBitmap> m_IconCache;

			wxString m_SearchMask;
			KxDataViewComboBoxEditor* m_PartOfEditor = nullptr;

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

			virtual bool GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const override;
			virtual bool HasDefaultCompare() const override
			{
				return true;
			}
			virtual bool Compare(const KxDataViewItem& item1, const KxDataViewItem& item2, const KxDataViewColumn* column) const override;

			void OnSelectItem(KxDataViewEvent& event);
			void OnActivateItem(KxDataViewEvent& event);
			void OnContextMenu(KxDataViewEvent& event);

		public:
			DisplayModel();

		public:
			virtual void RefreshItems() override;

			bool IsInSearchMode() const
			{
				return !m_SearchMask.IsEmpty();
			}
			bool SetSearchMask(const wxString& mask);

			KxDataViewItem MakeItem(const FileTreeNode* node) const
			{
				return KxDataViewItem(node);
			}
			KxDataViewItem MakeItem(const FileTreeNode& node) const
			{
				return MakeItem(&node);
			}
			FileTreeNode* GetNode(const KxDataViewItem& item) const
			{
				return item.GetValuePtr<FileTreeNode>();
			}
	};
}
