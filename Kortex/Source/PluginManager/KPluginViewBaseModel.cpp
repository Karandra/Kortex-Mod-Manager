#include "stdafx.h"
#include "KPluginViewBaseModel.h"
#include "KPluginViewModel.h"
#include "KPluginManagerWorkspace.h"
#include "KPluginReader.h"
#include "KPluginReaderBethesda.h"
#include "ModManager/KModManager.h"
#include "ModManager/KModEntry.h"
#include "ModManager/KModWorkspace.h"
#include "ModManager/KModManagerModel.h"
#include "Profile/KProfile.h"
#include "GameInstance/KGameInstance.h"
#include "GameInstance/Config/KPluginManagerConfig.h"
#include "UI/KMainWindow.h"
#include "UI/KTextEditorDialog.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxMenu.h>

enum ColumnID
{
	Name,
	Index,
	Type,
	PartOf,
	Author,
};

void KPluginViewBaseModel::OnInitControl()
{
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KPluginViewBaseModel::OnSelectItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KPluginViewBaseModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KPluginViewBaseModel::OnContextMenu, this);
	GetView()->Bind(KxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK, [this](KxDataViewEvent& event)
	{
		KxMenu menu;
		if (GetView()->CreateColumnSelectionMenu(menu))
		{
			GetView()->OnColumnSelectionMenu(menu);
		}
	});
	EnableDragAndDrop();

	KPluginViewModel* model = GetCoModel();
	model->SetView(GetView());
	model->OnInitControl();
}

void KPluginViewBaseModel::GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const
{
	if (item.IsTreeRootItem())
	{
		for (size_t i = 0; i < GetItemCount(); i++)
		{
			const KPluginEntry* entry = GetDataEntry(i);
			if (entry && KAux::CheckSearchMask(m_SearchMask, entry->GetName()))
			{
				children.push_back(GetItem(i));
			}
		}
	}
}
void KPluginViewBaseModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
{
	GetCoModel()->GetValue(value, *GetDataEntry(row), column);
}
bool KPluginViewBaseModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
{
	if (GetCoModel()->SetValue(value, *GetDataEntry(row), column))
	{
		ChangeNotify();
		return true;
	}
	return false;
}
bool KPluginViewBaseModel::IsEditorEnabledByRow(size_t row, const KxDataViewColumn* column) const
{
	return GetCoModel()->IsEditorEnabled(*GetDataEntry(row), column);
}
bool KPluginViewBaseModel::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
{
	const KPluginEntry* entry = GetDataEntry(row);
	if (entry && column->GetID() == ColumnID::Name)
	{
		return entry->CanToggleEnabled();
	}
	return true;
}
bool KPluginViewBaseModel::GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const
{
	return GetCoModel()->GetAttributes(*GetDataEntry(row), column, attributes, cellState);
}
bool KPluginViewBaseModel::CompareByRow(size_t row1, size_t row2, const KxDataViewColumn* column) const
{
	return GetCoModel()->Compare(*GetDataEntry(row1), *GetDataEntry(row2), column);
}

void KPluginViewBaseModel::OnSelectItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	const KPluginEntry* entry = GetDataEntry(GetRow(item));

	if (column && column->GetID() == ColumnID::PartOf)
	{
		KModWorkspace* workspace = KModWorkspace::GetInstance();
		wxWindowUpdateLocker lock(workspace);
		workspace->HighlightMod();

		if (entry && entry->GetParentMod())
		{
			workspace->HighlightMod(entry->GetParentMod());
		}
	}
	KPluginManagerWorkspace::GetInstance()->ProcessSelection(entry);
}
void KPluginViewBaseModel::OnActivateItem(KxDataViewEvent& event)
{
}
void KPluginViewBaseModel::OnContextMenu(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	const KPluginEntry* entry = GetDataEntry(GetRow(item));

	KxMenu menu;
	KPluginManager* manager = KPluginManager::GetInstance();
	KPluginManagerWorkspace* workspace = KPluginManagerWorkspace::GetInstance();

	// Base items
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(KTr("PluginManager.EnableAll")));
		item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
		{
			SetAllEnabled(true);
		});
	}
	{
		KxMenuItem* item = menu.Add(new KxMenuItem(KTr("PluginManager.DisableAll")));
		item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
		{
			SetAllEnabled(false);
		});
	}

	if (entry)
	{
		// Plugin reader dependent items
		if (const KPluginReader* reader = entry->GetReader())
		{
			const KPluginReaderBethesda* bethesdaReader = NULL;
			if (reader->As(bethesdaReader))
			{
				if (menu.GetMenuItemCount() != 0)
				{
					menu.AddSeparator();
				}

				// Description
				KxMenuItem* descriptionItem = menu.Add(new KxMenuItem(KTr("Generic.Description")));
				descriptionItem->Enable(!bethesdaReader->GetDescription().IsEmpty());
				descriptionItem->Bind(KxEVT_MENU_SELECT, [this, bethesdaReader](KxMenuEvent& event)
				{
					KTextEditorDialog dialog(GetView());
					dialog.SetText(bethesdaReader->GetDescription());
					dialog.SetEditable(false);
					dialog.ShowPreview(true);
					dialog.ShowModal();
				});

				menu.AddSeparator();

				// Dependencies
				const KxStringVector& dependenciesList = bethesdaReader->GetRequiredPlugins();
				KxMenu* dependenciesMenu = new KxMenu();
				KxMenuItem* dependenciesMenuItem = menu.Add(dependenciesMenu, wxString::Format("%s (%zu)", KTr("PluginManager.PluginDependencies"), dependenciesList.size()));
				dependenciesMenuItem->Enable(!dependenciesList.empty());

				for (const wxString& name: dependenciesList)
				{
					KxMenuItem* item = dependenciesMenu->Add(new KxMenuItem(name));
					item->SetBitmap(KGetBitmap(workspace->GetStatusImageForPlugin(manager->FindPluginByName(name))));
				}

				// Dependent plugins
				KPluginEntry::RefVector dependentList = manager->GetDependentPlugins(*entry);
				KxMenu* dependentMenu = new KxMenu();
				KxMenuItem* dependentMenuItem = menu.Add(dependentMenu, wxString::Format("%s (%zu)", KTr("PluginManager.DependentPlugins"), dependentList.size()));
				dependentMenuItem->Enable(!dependentList.empty());

				for (const KPluginEntry* depEntry: dependentList)
				{
					KxMenuItem* item = dependentMenu->Add(new KxMenuItem(depEntry->GetName()));
					item->SetBitmap(KGetBitmap(workspace->GetStatusImageForPlugin(depEntry)));
				}

				// Plugin select event
				auto OnSelectPlugin = [this](KxMenuEvent& event)
				{
					const KPluginEntry* entry = KPluginManager::GetInstance()->FindPluginByName(event.GetItem()->GetItemLabelText());
					SelectItem(GetItemByEntry(entry), true);
					GetView()->SetFocus();
				};
				dependenciesMenu->Bind(KxEVT_MENU_SELECT, OnSelectPlugin);
				dependentMenu->Bind(KxEVT_MENU_SELECT, OnSelectPlugin);
			}
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

bool KPluginViewBaseModel::OnDragItems(KxDataViewEventDND& event)
{
	if (CanDragDropNow())
	{
		if (const KPluginEntry* entry = GetDataEntry(GetRow(event.GetItem())))
		{
			KxDataViewItem::Vector selected;
			if (GetView()->GetSelections(selected) > 0)
			{
				std::unique_ptr<DragDropDataObjectT> dataObject;
				for (const auto& item: selected)
				{
					if (KPluginEntry* entry = GetDataEntry(GetRow(item)))
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
bool KPluginViewBaseModel::OnDropItems(KxDataViewEventDND& event)
{
	const KPluginEntry* entry = GetDataEntry(GetRow(event.GetItem()));
	if (entry && HasDragDropDataObject())
	{
		const KPluginEntry::RefVector& entriesToMove = GetDragDropDataObject()->GetEntries();
		
		// Move and refresh
		if (KPluginManager::GetInstance()->MovePluginsIntoThis(entriesToMove, *entry))
		{
			ChangeNotify();
			RefreshItems();

			// Select moved items and event-select the first one
			for (KPluginEntry* entry: entriesToMove)
			{
				GetView()->Select(GetItemByEntry(entry));
			}
			SelectItem(GetItemByEntry(entriesToMove.front()));

			KEvent::MakeSend<KPluginEvent>(KEVT_PLUGINS_REORDERED, entriesToMove);
			return true;
		}
	}
	return false;
}
bool KPluginViewBaseModel::CanDragDropNow() const
{
	if (KxDataViewColumn* column = GetView()->GetSortingColumn())
	{
		return column->GetID() == ColumnID::Index && column->IsSortedAscending();
	}
	return true;
}

KPluginViewBaseModel::KPluginViewBaseModel()
	:m_Entries(KPluginManager::GetInstance()->GetEntries())
{
	SetDataViewFlags(KxDataViewCtrl::DefaultStyle|KxDV_MULTIPLE_SELECTION|KxDV_NO_TIMEOUT_EDIT|KxDV_VERT_RULES);
}

void KPluginViewBaseModel::ChangeNotify()
{
	if (KProfile* profile = KGameInstance::GetActiveProfile())
	{
		profile->SyncWithCurrentState();
		profile->Save();
	}
	KPluginManager::GetInstance()->Save();
}
void KPluginViewBaseModel::SetDataVector()
{
	KDataViewVectorListModel::SetDataVector();
}
void KPluginViewBaseModel::SetDataVector(KPluginEntry::Vector& array)
{
	KDataViewVectorListModel::SetDataVector(&m_Entries);
}
KPluginViewModel* KPluginViewBaseModel::GetCoModel() const
{
	return KPluginManager::GetInstance()->GetViewModel();
}

void KPluginViewBaseModel::SetAllEnabled(bool value)
{
	KPluginManager::GetInstance()->SetAllPluginsEnabled(value);
	GetView()->Refresh();
	ChangeNotify();
}
void KPluginViewBaseModel::UpdateUI()
{
	GetView()->Refresh();
}
