#pragma once
#include "stdafx.h"
#include <Kortex/ModManager.hpp>
#include "Utility/KDataViewListModel.h"
#include "Utility/KImageProvider.h"

namespace Kortex
{
	class IGameMod;
	class FileTreeNode;
}

namespace Kortex::ModManager
{
	class KModFilesExplorerModel: public KxDataViewModelExBase<KxDataViewModel>
	{
		private:
			const IGameMod& m_ModEntry;
			std::unordered_map<const FileTreeNode*, KDispatcherCollision::Vector> m_Collisions;

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

			const KDispatcherCollision::Vector* GetCollisions(const FileTreeNode& node) const;
			const KDispatcherCollision::Vector& GetOrUpdateCollisions(const FileTreeNode& node);

		public:
			KModFilesExplorerModel(const IGameMod& modEntry)
				:m_ModEntry(modEntry)
			{
			}

		public:
			virtual void RefreshItems() override;

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
