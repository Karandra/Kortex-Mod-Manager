#pragma once
#include "stdafx.h"
#include <Kortex/ModManager.hpp>
#include "KxFramework/KxDataViewListModelEx.h"

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
			void OnInitControl() override;
			bool IsContainer(const KxDataViewItem& item) const override;
			bool HasContainerColumns(const KxDataViewItem& item) const override
			{
				return true;
			}
			KxDataViewItem GetParent(const KxDataViewItem& item) const override;
			void GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const override;
		
			void GetEditorValue(wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			void GetValue(wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			bool SetValue(const wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column) override;
			bool IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const override;

			bool GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const override;
			bool HasDefaultCompare() const override
			{
				return true;
			}
			bool Compare(const KxDataViewItem& item1, const KxDataViewItem& item2, const KxDataViewColumn* column) const override;

			void OnSelectItem(KxDataViewEvent& event);
			void OnActivateItem(KxDataViewEvent& event);
			void OnContextMenu(KxDataViewEvent& event);

		public:
			DisplayModel();

		public:
			void RefreshItems() override;
			void ClearView();

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
