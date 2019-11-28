#pragma once
#include "stdafx.h"
#include <KxFramework/KxDataViewListModelEx.h>
class KxMenu;
class KxMenuItem;

namespace Kortex::PackageDesigner
{
	class KPackageProject;
	class WorkspaceDocument;
	class IDTracker;
}

namespace Kortex::PackageDesigner
{
	class ListModelDataObject;
	class ListModel: public KxDataViewListModelEx, public KxDataViewModelExDragDropEnabled<ListModelDataObject>
	{
		protected:
			WorkspaceDocument* m_Controller = nullptr;

		protected:
			/* ID tracker */
			virtual IDTracker* GetTracker()
			{
				return nullptr;
			}

			/* All items menu */
			KxMenu* CreateAllItemsMenu(KxMenu& menu);
			KxMenuItem* CreateAllItemsMenuEntry(KxMenu* menu, int columnID);
			virtual void OnAllItemsMenuSelect(KxDataViewColumn* column) {};

		protected:
			/* Drag and Drop */
			virtual KxDataViewCtrl* GetViewCtrl() const override
			{
				return GetView();
			}
			virtual bool OnDragItems(KxDataViewEventDND& event) override;
			virtual bool OnDropItems(KxDataViewEventDND& event) override;
			virtual bool OnInsertItem(KxDataViewItem& currentItem, KxDataViewItem& droppedItem)
			{
				return false;
			}

			template<class T> void OnInsertItemHelperPrimitive(T& itemsList, KxDataViewItem& currentItem, KxDataViewItem& droppedItem)
			{
				auto itCurrent = itemsList.begin() + GetRow(currentItem);
				auto itDropped = itemsList.begin() + GetRow(droppedItem);

				auto entry = *itDropped;
				itemsList.erase(itDropped);
				itemsList.emplace(itCurrent, entry);
			}
			template<class T> void OnInsertItemHelperUniquePtr(T& itemsList, KxDataViewItem& currentItem, KxDataViewItem& droppedItem)
			{
				auto itCurrent = itemsList.begin() + GetRow(currentItem);
				auto itDropped = itemsList.begin() + GetRow(droppedItem);

				auto entry = itDropped->release();
				itemsList.erase(itDropped);
				itemsList.emplace(itCurrent, entry);
			}

		public:
			ListModel();
			void Create(wxWindow* window, wxSizer* sizer) = delete;
			virtual void Create(WorkspaceDocument* controller, wxWindow* window, wxSizer* sizer);
			virtual ~ListModel();

		public:
			/* Misc */
			KPackageProject& GetProject() const;
			wxRect GetItemRect(const KxDataViewItem& item, const KxDataViewColumn* column) const
			{
				return GetView()->GetAdjustedItemRect(item, column);
			}

			/* Changes */
			virtual void ChangeNotify();
			void NotifyChangedItem(const KxDataViewItem& item);
			void NotifyAddedItem(const KxDataViewItem& item);
			void NotifyRemovedItem(const KxDataViewItem& item);
			void NotifyAllItemsChanged();
			void NotifyCleared();

			template<class T> void RemoveItemAndNotify(T& itemsList, const KxDataViewItem& item)
			{
				size_t index = GetRow(item);
				if (index < GetItemCount())
				{
					itemsList.erase(itemsList.begin() + index);
					NotifyRemovedItem(item);
				}
			}
			template<class T> void ClearItemsAndNotify(T& itemsList)
			{
				itemsList.clear();
				NotifyCleared();
			}
	};
}

namespace Kortex::PackageDesigner
{
	class ListModelDataObject: public KxDataViewModelExDragDropData
	{
		private:
			KxDataViewItem m_Item;

		public:
			ListModelDataObject(const KxDataViewItem& item)
				:m_Item(item)
			{
			}

		public:
			KxDataViewItem GetItem() const
			{
				return m_Item;
			}
	};
}
