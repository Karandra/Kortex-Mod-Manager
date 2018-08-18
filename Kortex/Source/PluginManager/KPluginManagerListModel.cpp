#include "stdafx.h"
#include "KPluginManagerListModel.h"
#include "KPluginManagerWorkspace.h"
#include "ModManager/KModManager.h"
#include "ModManager/KModEntry.h"
#include "ModManager/KModManagerWorkspace.h"
#include "ModManager/KModManagerModel.h"
#include "Profile/KPluginManagerConfig.h"
#include "UI/KMainWindow.h"
#include "UI/KTextEditorDialog.h"
#include "KApp.h"
#include "KAux.h"
#include "KComparator.h"
#include <KxFramework/KxMenu.h>

enum ColumnID
{
	Name,
	Index,
	Type,
	PartOf,
	Author,
};

void KPluginManagerListModel::OnInitControl()
{
	/* View */
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KPluginManagerListModel::OnSelectItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KPluginManagerListModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KPluginManagerListModel::OnContextMenu, this);
	GetView()->Bind(KxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK, [this](KxDataViewEvent& event)
	{
		KxMenu menu;
		if (GetView()->CreateColumnSelectionMenu(menu))
		{
			GetView()->OnColumnSelectionMenu(menu);
		}
	});
	EnableDragAndDrop();

	/* Columns */
	KxDataViewColumnFlags defaultFlags = KxDV_COL_DEFAULT_FLAGS|KxDV_COL_SORTABLE;

	GetView()->AppendColumn<KxDataViewBitmapTextToggleRenderer>(T("PluginManager.List.Name"), ColumnID::Name, KxDATAVIEW_CELL_ACTIVATABLE, KxDVC_DEFAULT_WIDTH, defaultFlags);
	{
		auto info = GetView()->AppendColumn<KxDataViewTextRenderer>(T("PluginManager.List.Index"), ColumnID::Index, KxDATAVIEW_CELL_INERT, KxDVC_DEFAULT_WIDTH, defaultFlags);
		info.GetColumn()->SortAscending();
	}
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("PluginManager.List.Type"), ColumnID::Type, KxDATAVIEW_CELL_INERT, KxDVC_DEFAULT_WIDTH, defaultFlags);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("PluginManager.List.PartOf"), ColumnID::PartOf, KxDATAVIEW_CELL_INERT, KxDVC_DEFAULT_WIDTH, defaultFlags);
	GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.Author"), ColumnID::Author, KxDATAVIEW_CELL_INERT, KxDVC_DEFAULT_WIDTH, defaultFlags);
}

void KPluginManagerListModel::GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const
{
	if (item.IsTreeRootItem())
	{
		for (size_t i = 0; i < GetItemCount(); i++)
		{
			const KPMPluginEntry* entry = GetDataEntry(i);
			if (entry && KAux::CheckSearchMask(m_SearchMask, entry->GetName()))
			{
				children.push_back(GetItem(i));
			}
		}
	}
}
void KPluginManagerListModel::GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const
{
	const KPMPluginEntry* entry = GetDataEntry(row);
	switch (column->GetID())
	{
		case ColumnID::Name:
		{
			data = KxDataViewBitmapTextToggleValue(entry->IsEnabled(), entry->GetName(), wxNullBitmap, KxDataViewBitmapTextToggleValue::CheckBox);
			break;
		}
		case ColumnID::Index:
		{
			if (entry->GetFormat() & KPMPE_TYPE_LIGHT)
			{
				data = wxString::Format("0x%02X::%04d", ms_LightPluginIndex, CountLightActiveBefore(row));
			}
			else
			{
				int index = 0;
				if (GetPluginIndex(entry, index))
				{
					data = wxString::Format("0x%02X (%d)", index, index);
				}
			}
			break;
		}
		case ColumnID::Type:
		{
			data = KPluginManager::GetInstance()->GetPluginTypeName(entry->GetFormat());
			break;
		}
		case ColumnID::PartOf:
		{
			data = GetPartOfName(entry);
			break;
		}
		case ColumnID::Author:
		{
			data = GetPluginAuthor(entry);
			break;
		}
	};
}
bool KPluginManagerListModel::SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column)
{
	KPMPluginEntry* entry = GetDataEntry(row);
	if (entry)
	{
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				entry->SetEnabled(data.As<bool>());
				KPluginManager::GetInstance()->UpdateAllPlugins();
				ChangeNotify();
				return true;
			}
		};
	}
	return false;
}
bool KPluginManagerListModel::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
{
	const KPMPluginEntry* entry = GetDataEntry(row);
	if (entry && column->GetID() == ColumnID::Name)
	{
		return entry->CanToggleEnabled();
	}
	return true;
}
bool KPluginManagerListModel::GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const
{
	if (const KPMPluginEntry* entry = GetDataEntry(row))
	{
		switch (column->GetID())
		{
			case ColumnID::PartOf:
			{
				const KModEntry* modEntry = entry->GetParentMod();
				if (modEntry)
				{
					if (cellState & KxDATAVIEW_CELL_HIGHLIGHTED && column->IsHotTracked())
					{
						attributes.SetUnderlined(true);
					}

					const KPluginManagerConfigStandardContentEntry* pStandardContentEntry = entry->GetStdContentEntry();
					if (modEntry->ToFixedEntry() && !pStandardContentEntry)
					{
						attributes.SetItalic(true);
					}
				}
				break;
			}
		};
	}
	return !attributes.IsDefault();
}
bool KPluginManagerListModel::CompareByRow(size_t row1, size_t row2, const KxDataViewColumn* column) const
{
	const KPMPluginEntry* pEntry1 = GetDataEntry(row1);
	const KPMPluginEntry* pEntry2 = GetDataEntry(row2);
	using KComparator::KCompare;

	switch (column->GetID())
	{
		case ColumnID::Name:
		{
			return KCompare(pEntry1->GetName(), pEntry2->GetName()) < 0;
		}
		default:
		case ColumnID::Index:
		{
			KPluginManager* manager = KPluginManager::GetInstance();
			return manager->GetPluginIndex(pEntry1) < manager->GetPluginIndex(pEntry2);
		}
		case ColumnID::Type:
		{
			return -pEntry1->GetFormat() < -pEntry2->GetFormat();
			break;
		}
		case ColumnID::PartOf:
		{
			return KCompare(GetPartOfName(pEntry1), GetPartOfName(pEntry2)) < 0;
		}
		case ColumnID::Author:
		{
			return KCompare(GetPluginAuthor(pEntry1), GetPluginAuthor(pEntry2)) < 0;
		}
	};
	return false;
}

bool KPluginManagerListModel::GetPluginIndex(const KPMPluginEntry* entry, int& index) const
{
	if (entry->IsEnabled())
	{
		index = GetRow(GetItemByEntry(entry));
		if (entry->GetFormat() & KPMPE_TYPE_LIGHT)
		{
			index = ms_LightPluginIndex;
		}
		else
		{
			index = index - CountInactiveBefore(index);
		}
		return true;
	}
	return false;
}
int KPluginManagerListModel::CountInactiveBefore(size_t index) const
{
	int count = 0;
	for (size_t i = 0; i < std::min(GetDataVector()->size(), index); i++)
	{
		const KPMPluginEntry* entry = GetDataVector()->at(i).get();
		if (!entry->IsEnabled() || entry->GetFormat() & KPMPE_TYPE_LIGHT)
		{
			count++;
		}
		if (i == index)
		{
			break;
		}
	}
	return count;
}
int KPluginManagerListModel::CountLightActiveBefore(size_t index) const
{
	int count = 0;
	for (size_t i = 0; i < std::min(GetDataVector()->size(), index); i++)
	{
		const KPMPluginEntry* entry = GetDataVector()->at(i).get();
		if (entry->GetFormat() & KPMPE_TYPE_LIGHT && entry->IsEnabled())
		{
			count++;
		}
		if (i == index)
		{
			break;
		}
	}
	return count;
}
const wxString& KPluginManagerListModel::GetPartOfName(const KPMPluginEntry* entry) const
{
	if (const KPluginManagerConfigStandardContentEntry* pStandardContentEntry = entry->GetStdContentEntry())
	{
		return pStandardContentEntry->GetName();
	}
	else if (const KModEntry* modEntry = entry->GetParentMod())
	{
		return modEntry->GetName();
	}
	return wxNullString;
}
wxString KPluginManagerListModel::GetPluginAuthor(const KPMPluginEntry* entry) const
{
	if (KPMPluginReader* reader = entry->GetPluginReader())
	{
		return reader->GetAuthor();
	}
	return wxEmptyString;
}

void KPluginManagerListModel::OnSelectItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	const KPMPluginEntry* entry = GetDataEntry(GetRow(item));

	if (column && column->GetID() == ColumnID::PartOf)
	{
		KModManagerWorkspace* workspace = KModManagerWorkspace::GetInstance();
		wxWindowUpdateLocker lock(workspace);
		workspace->HighlightMod();

		if (entry && entry->GetParentMod())
		{
			workspace->HighlightMod(entry->GetParentMod());
		}
	}
	GetWorkspace()->ProcessSelection(entry);
}
void KPluginManagerListModel::OnActivateItem(KxDataViewEvent& event)
{
}
void KPluginManagerListModel::OnContextMenu(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	const KPMPluginEntry* entry = GetDataEntry(GetRow(item));

	KxMenu menu;
	KPluginManager* manager = KPluginManager::GetInstance();
	KPluginManagerWorkspace* workspace = KPluginManagerWorkspace::GetInstance();

	// Base items
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(T("PluginManager.EnableAll")));
		item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
		{
			SetAllEnabled(true);
		});
	}
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(T("PluginManager.DisableAll")));
		item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
		{
			SetAllEnabled(false);
		});
	}

	if (entry)
	{
		// Plugin reader dependent items
		if (KPMPluginReader* reader = entry->GetPluginReader())
		{
			if (menu.GetMenuItemCount() != 0)
			{
				menu.AddSeparator();
			}

			// Description
			KxMenuItem* pDescriptionItem = menu.Add(new KxMenuItem(T("Generic.Description")));
			pDescriptionItem->Enable(!reader->GetDescription().IsEmpty());
			pDescriptionItem->Bind(KxEVT_MENU_SELECT, [this, reader](KxMenuEvent& event)
			{
				KTextEditorDialog dialog(GetView());
				dialog.SetText(reader->GetDescription());
				dialog.SetEditable(false);
				dialog.ShowPreview(true);
				dialog.ShowModal();
			});

			menu.AddSeparator();

			// Dependencies
			const KxStringVector& dependenciesList = reader->GetDependencies();
			KxMenu* dependenciesMenu = new KxMenu();
			KxMenuItem* dependenciesMenuItem = menu.Add(dependenciesMenu, wxString::Format("%s (%zu)", T("PluginManager.PluginDependencies"), dependenciesList.size()));
			dependenciesMenuItem->Enable(!dependenciesList.empty());

			for (const wxString& name: dependenciesList)
			{
				KxMenuItem* item = dependenciesMenu->Add(new KxMenuItem(name));
				item->SetBitmap(KGetBitmap(workspace->GetStatusImageForPlugin(manager->FindPluginByName(name))));
			}

			// Dependent plugins
			KPMPluginEntryRefVector dependentList = manager->GetDependentPlugins(entry);
			KxMenu* dependentMenu = new KxMenu();
			KxMenuItem* dependentMenuItem = menu.Add(dependentMenu, wxString::Format("%s (%zu)", T("PluginManager.DependentPlugins"), dependentList.size()));
			dependentMenuItem->Enable(!dependentList.empty());

			for (const KPMPluginEntry* depEntry: dependentList)
			{
				KxMenuItem* item = dependentMenu->Add(new KxMenuItem(depEntry->GetName()));
				item->SetBitmap(KGetBitmap(workspace->GetStatusImageForPlugin(depEntry)));
			}

			// Plugin select event
			auto OnSelectPlugin = [this](KxMenuEvent& event)
			{
				const KPMPluginEntry* entry = KPluginManager::GetInstance()->FindPluginByName(event.GetItem()->GetItemLabelText());
				SelectItem(GetItemByEntry(entry), true);
				GetView()->SetFocus();
			};
			dependenciesMenu->Bind(KxEVT_MENU_SELECT, OnSelectPlugin);
			dependentMenu->Bind(KxEVT_MENU_SELECT, OnSelectPlugin);
		}
	}

	// Workspace specific menus
	if (menu.GetMenuItemCount() != 0)
	{
		menu.AddSeparator();
	}

	workspace->OnCreateViewContextMenu(menu, entry);
	workspace->OnCreateSortingToolsMenu(menu, entry);
	workspace->OnCreateImportExportMenu(menu, entry);

	// Show
	menu.Show(GetView());
}

bool KPluginManagerListModel::OnDragItems(KxDataViewEventDND& event)
{
	if (CanDragDropNow())
	{
		if (const KPMPluginEntry* entry = GetDataEntry(GetRow(event.GetItem())))
		{
			KxDataViewItem::Vector selected;
			if (GetView()->GetSelections(selected) > 0)
			{
				std::unique_ptr<DragDropDataObjectT> dataObject;
				for (const auto& item: selected)
				{
					if (KPMPluginEntry* entry = GetDataEntry(GetRow(item)))
					{
						if (!dataObject)
						{
							dataObject = std::make_unique<DragDropDataObjectT>(selected.size());
						}
						dataObject->AddEntry(entry);
					}
				}

				if (dataObject)
				{
					SetDragDropDataObject(dataObject.release());
					event.SetDragFlags(wxDrag_AllowMove);
					return true;
				}
			}
		}
	}
	return false;
}
bool KPluginManagerListModel::OnDropItems(KxDataViewEventDND& event)
{
	const KPMPluginEntry* entry = GetDataEntry(GetRow(event.GetItem()));
	if (entry && HasDragDropDataObject())
	{
		const KPMPluginEntryRefVector& entriesToMove = GetDragDropDataObject()->GetEntries();
		
		// Move and refresh
		if (KPluginManager::GetInstance()->MovePluginsIntoThis(entriesToMove, entry))
		{
			ChangeNotify();
			RefreshItems();

			// Select moved items and Event-select the first one
			for (KPMPluginEntry* entry: entriesToMove)
			{
				GetView()->Select(GetItemByEntry(entry));
			}

			SelectItem(GetItemByEntry(entriesToMove.front()));
			return true;
		}
	}
	return false;
}
bool KPluginManagerListModel::CanDragDropNow() const
{
	if (KxDataViewColumn* column = GetView()->GetSortingColumn())
	{
		return column->GetID() == ColumnID::Index && column->IsSortedAscending();
	}
	return true;
}

KPluginManagerListModel::KPluginManagerListModel(KPluginManagerWorkspace* workspace)
	:m_Workspace(workspace)
{
	SetDataViewFlags(KxDataViewCtrl::DefaultStyle|KxDV_MULTIPLE_SELECTION|KxDV_NO_TIMEOUT_EDIT|KxDV_VERT_RULES);
}

void KPluginManagerListModel::ChangeNotify()
{
	KModManager::GetListManager().SyncCurrentList();
	KPluginManager::GetInstance()->Save();
}

void KPluginManagerListModel::SetDataVector()
{
	KDataViewVectorListModel::SetDataVector();
}
void KPluginManagerListModel::SetDataVector(KPMPluginEntryVector& array)
{
	KDataViewVectorListModel::SetDataVector(&array);
}
void KPluginManagerListModel::SetAllEnabled(bool value)
{
	KPluginManager::GetInstance()->SetAllPluginsEnabled(value);
	KPluginManager::GetInstance()->UpdateAllPlugins();

	for (size_t i = 0; i < GetItemCount(); i++)
	{
		ItemChanged(GetItem(i));
	}
	ChangeNotify();
}
void KPluginManagerListModel::UpdateUI()
{
	SelectItem(GetView()->GetSelection());
}
