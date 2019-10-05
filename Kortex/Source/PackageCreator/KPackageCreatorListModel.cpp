#include "stdafx.h"
#include "KPackageCreatorListModel.h"
#include "KPackageCreatorController.h"
#include "KPackageCreatorPageBase.h"
#include "PackageProject/KPackageProject.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxString.h>

namespace Kortex::PackageDesigner
{
	KxMenu* KPackageCreatorListModel::CreateAllItemsMenu(KxMenu& menu)
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
	KxMenuItem* KPackageCreatorListModel::CreateAllItemsMenuEntry(KxMenu* menu, int columnID)
	{
		KxDataViewColumn* column = GetView()->GetColumn(columnID);
		KxMenuItem* item = menu->Add(new KxMenuItem(column->GetTitle()));
		item->SetClientData(column);

		return item;
	}

	bool KPackageCreatorListModel::OnDragItems(KxDataViewEventDND& event)
	{
		if (GetView()->GetSelectedItemsCount() == 1)
		{
			SetDragDropDataObject(new KPackageCreatorListModelDataObject(GetView()->GetSelection()));
			return true;
		}
		return false;
	}
	bool KPackageCreatorListModel::OnDropItems(KxDataViewEventDND& event)
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

	KPackageCreatorListModel::KPackageCreatorListModel()
	{
	}
	void KPackageCreatorListModel::Create(KPackageCreatorController* controller, wxWindow* window, wxSizer* sizer)
	{
		m_Controller = controller;

		SetDataViewFlags(GetDataViewFlags()|KxDV_NO_TIMEOUT_EDIT|KxDV_VERT_RULES);
		KDataViewListModel::Create(window, sizer);
		EnableDragAndDrop();
	}
	KPackageCreatorListModel::~KPackageCreatorListModel()
	{
	}

	KPackageProject& KPackageCreatorListModel::GetProject() const
	{
		return *m_Controller->GetProject();
	}

	void KPackageCreatorListModel::ChangeNotify()
	{
		m_Controller->ChangeNotify();
	}
	void KPackageCreatorListModel::NotifyChangedItem(const KxDataViewItem& item)
	{
		ItemChanged(item);
		ChangeNotify();
	}
	void KPackageCreatorListModel::NotifyAddedItem(const KxDataViewItem& item)
	{
		if (item.IsOK())
		{
			ItemAdded(KxDataViewItem(), item);
			SelectItem(item);
			ChangeNotify();
		}
	}
	void KPackageCreatorListModel::NotifyRemovedItem(const KxDataViewItem& item)
	{
		if (item.IsOK())
		{
			size_t index = GetRow(item);

			RefreshItems();
			SelectItem(index == 0 ? 0 : index - 1);
			ChangeNotify();
		}
	}
	void KPackageCreatorListModel::NotifyAllItemsChanged()
	{
		for (size_t i = 0; i < GetItemCount(); i++)
		{
			ItemChanged(GetItem(i));
		}
		ChangeNotify();
	}
	void KPackageCreatorListModel::NotifyCleared()
	{
		RefreshItems();
		ChangeNotify();
		SelectItem(KxDataViewItem());
	}
}
