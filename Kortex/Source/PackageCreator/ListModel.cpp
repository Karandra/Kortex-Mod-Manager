#include "stdafx.h"
#include "ListModel.h"
#include "WorkspaceDocument.h"
#include "PageBase.h"
#include "PackageProject/KPackageProject.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxString.h>

namespace Kortex::PackageDesigner
{
	KxMenu* ListModel::CreateAllItemsMenu(KxMenu& menu)
	{
		KxMenu* subMenu = new KxMenu();
		KxMenuItem* item = menu.Add(subMenu, KTr("Generic.All"));
		item->Enable(!IsEmpty());

		subMenu->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
		{
			if (void* clientData = event.GetItem()->GetClientData())
			{
				OnAllItemsMenuSelect(static_cast<KxDataViewColumn*>(clientData));
			}
			event.Skip();
		});
		return subMenu;
	}
	KxMenuItem* ListModel::CreateAllItemsMenuEntry(KxMenu* menu, int columnID)
	{
		KxDataViewColumn* column = GetView()->GetColumn(columnID);
		KxMenuItem* item = menu->Add(new KxMenuItem(column->GetTitle()));
		item->SetClientData(column);

		return item;
	}

	bool ListModel::OnDragItems(KxDataViewEventDND& event)
	{
		if (GetView()->GetSelectedItemsCount() == 1)
		{
			SetDragDropDataObject(new ListModelDataObject(GetView()->GetSelection()));
			return true;
		}
		return false;
	}
	bool ListModel::OnDropItems(KxDataViewEventDND& event)
	{
		if (HasDragDropDataObject())
		{
			KxDataViewItem currentItem = event.GetItem();
			KxDataViewItem droppedItem = GetDragDropDataObject()->GetItem();
			if (currentItem.IsOK() && droppedItem.IsOK() && currentItem != droppedItem)
			{
				if (OnInsertItem(currentItem, droppedItem))
				{
					NotifyChangedItem(currentItem);
					NotifyChangedItem(droppedItem);

					if (GetView()->GetSelection() != currentItem)
					{
						SelectItem(currentItem);
					}
					return true;
				}
			}
		}
		return false;
	}

	ListModel::ListModel()
	{
	}
	void ListModel::Create(WorkspaceDocument* controller, wxWindow* window, wxSizer* sizer)
	{
		m_Controller = controller;

		SetDataViewFlags(GetDataViewFlags()|KxDV_NO_TIMEOUT_EDIT|KxDV_VERT_RULES);
		KDataViewListModel::Create(window, sizer);
		EnableDragAndDrop();
	}
	ListModel::~ListModel()
	{
	}

	ModPackageProject& ListModel::GetProject() const
	{
		return *m_Controller->GetProject();
	}

	void ListModel::ChangeNotify()
	{
		m_Controller->ChangeNotify();
	}
	void ListModel::NotifyChangedItem(const KxDataViewItem& item)
	{
		ItemChanged(item);
		ChangeNotify();
	}
	void ListModel::NotifyAddedItem(const KxDataViewItem& item)
	{
		if (item.IsOK())
		{
			ItemAdded(KxDataViewItem(), item);
			SelectItem(item);
			ChangeNotify();
		}
	}
	void ListModel::NotifyRemovedItem(const KxDataViewItem& item)
	{
		if (item.IsOK())
		{
			size_t index = GetRow(item);

			RefreshItems();
			SelectItem(index == 0 ? 0 : index - 1);
			ChangeNotify();
		}
	}
	void ListModel::NotifyAllItemsChanged()
	{
		for (size_t i = 0; i < GetItemCount(); i++)
		{
			ItemChanged(GetItem(i));
		}
		ChangeNotify();
	}
	void ListModel::NotifyCleared()
	{
		RefreshItems();
		ChangeNotify();
		SelectItem(KxDataViewItem());
	}
}
