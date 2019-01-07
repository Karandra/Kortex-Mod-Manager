#include "stdafx.h"
#include <Kortex/PluginManager.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/Application.hpp>
#include "UI/KMainWindow.h"
#include "UI/KTextEditorDialog.h"
#include "Utility/KAux.h"
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxMenu.h>

namespace Kortex::PluginManager
{
	void PluginViewModel::OnInitControl()
	{
		m_DisplayModel = IPluginManager::GetInstance()->CreateDisplayModel();

		GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &PluginViewModel::OnSelectItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &PluginViewModel::OnActivateItem, this);
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &PluginViewModel::OnContextMenu, this);
		GetView()->Bind(KxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK, [this](KxDataViewEvent& event)
		{
			KxMenu menu;
			if (GetView()->CreateColumnSelectionMenu(menu))
			{
				GetView()->OnColumnSelectionMenu(menu);
			}
		});
		EnableDragAndDrop();

		IDisplayModel* model = GetDisplayModel();
		model->SetView(GetView());
		model->OnInitControl();
	}

	void PluginViewModel::GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const
	{
		if (item.IsTreeRootItem())
		{
			for (size_t i = 0; i < GetItemCount(); i++)
			{
				const IGamePlugin* plugin = GetDataEntry(i);
				if (plugin && KAux::CheckSearchMask(m_SearchMask, plugin->GetName()))
				{
					children.push_back(GetItem(i));
				}
			}
		}
	}
	void PluginViewModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		if (const IGamePlugin* plugin = GetDataEntry(row))
		{
			GetDisplayModel()->GetValue(value, *plugin, column);
		}
	}
	bool PluginViewModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
	{
		IGamePlugin* plugin = GetDataEntry(row);
		if (plugin && GetDisplayModel()->SetValue(value, *plugin, column))
		{
			ChangeNotify();
			return true;
		}
		return false;
	}
	bool PluginViewModel::IsEditorEnabledByRow(size_t row, const KxDataViewColumn* column) const
	{
		if (const IGamePlugin* plugin = GetDataEntry(row))
		{
			return GetDisplayModel()->IsEditorEnabled(*plugin, column);
		}
		return false;
	}
	bool PluginViewModel::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
	{
		if (const IGamePlugin* plugin = GetDataEntry(row))
		{
			return GetDisplayModel()->IsEnabled(*plugin, column);
		}
		return false;
	}
	bool PluginViewModel::GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const
	{
		if (const IGamePlugin* plugin = GetDataEntry(row))
		{
			return GetDisplayModel()->GetAttributes(*plugin, column, attributes, cellState);
		}
		return false;
	}
	bool PluginViewModel::CompareByRow(size_t row1, size_t row2, const KxDataViewColumn* column) const
	{
		const IGamePlugin* plugin1 = GetDataEntry(row1);
		const IGamePlugin* plugin2 = GetDataEntry(row2);
		if (plugin1 && plugin2)
		{
			return GetDisplayModel()->Compare(*plugin1, *plugin2, column);
		}
		return false;
	}

	void PluginViewModel::OnSelectItem(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		KxDataViewColumn* column = event.GetColumn();
		const IGamePlugin* entry = GetDataEntry(GetRow(item));

		if (column && column->GetID() == IDisplayModel::ColumnID::PartOf)
		{
			ModManager::Workspace* workspace = ModManager::Workspace::GetInstance();
			wxWindowUpdateLocker lock(workspace);
			workspace->HighlightMod();

			if (entry && entry->GetOwningMod())
			{
				workspace->HighlightMod(entry->GetOwningMod());
			}
		}
		Workspace::GetInstance()->ProcessSelection(entry);
	}
	void PluginViewModel::OnActivateItem(KxDataViewEvent& event)
	{
	}
	void PluginViewModel::OnContextMenu(KxDataViewEvent& event)
	{
		KxDataViewItem item = event.GetItem();
		KxDataViewColumn* column = event.GetColumn();
		const IGamePlugin* entry = GetDataEntry(GetRow(item));

		KxMenu menu;
		IPluginManager* manager = IPluginManager::GetInstance();
		Workspace* workspace = Workspace::GetInstance();

		// Base items
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(KTr("PluginManager.ActivateAll")));
			item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
			{
				SetAllEnabled(true);
			});
		}
		{
			KxMenuItem* item = menu.Add(new KxMenuItem(KTr("PluginManager.DeactivateAll")));
			item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
			{
				SetAllEnabled(false);
			});
		}

		const IBethesdaGamePlugin* bethesdaPlugin = nullptr;
		if (entry && entry->QueryInterface(bethesdaPlugin))
		{
			if (menu.GetMenuItemCount() != 0)
			{
				menu.AddSeparator();
			}

			// Description
			KxMenuItem* descriptionItem = menu.Add(new KxMenuItem(KTr("Generic.Description")));
			descriptionItem->Enable(!bethesdaPlugin->GetDescription().IsEmpty());
			descriptionItem->Bind(KxEVT_MENU_SELECT, [this, bethesdaPlugin](KxMenuEvent& event)
			{
				KTextEditorDialog dialog(GetView());
				dialog.SetText(bethesdaPlugin->GetDescription());
				dialog.SetEditable(false);
				dialog.ShowPreview(true);
				dialog.ShowModal();
			});

			menu.AddSeparator();

			// Dependencies
			const KxStringVector& dependenciesList = bethesdaPlugin->GetRequiredPlugins();
			KxMenu* dependenciesMenu = new KxMenu();
			KxMenuItem* dependenciesMenuItem = menu.Add(dependenciesMenu, wxString::Format("%s (%zu)", KTr("PluginManager.PluginDependencies"), dependenciesList.size()));
			dependenciesMenuItem->Enable(!dependenciesList.empty());

			for (const wxString& name: dependenciesList)
			{
				KxMenuItem* item = dependenciesMenu->Add(new KxMenuItem(name));
				item->SetBitmap(KGetBitmap(workspace->GetStatusImageForPlugin(manager->FindPluginByName(name))));
			}

			// Dependent plugins
			IGamePlugin::RefVector dependentList = entry->GetDependentPlugins();
			KxMenu* dependentMenu = new KxMenu();
			KxMenuItem* dependentMenuItem = menu.Add(dependentMenu, KxString::Format("%1 (%2)", KTr("PluginManager.DependentPlugins"), dependentList.size()));
			dependentMenuItem->Enable(!dependentList.empty());

			for (const IGamePlugin* depEntry: dependentList)
			{
				KxMenuItem* item = dependentMenu->Add(new KxMenuItem(depEntry->GetName()));
				item->SetBitmap(KGetBitmap(workspace->GetStatusImageForPlugin(depEntry)));
			}

			// Plugin select event
			auto OnSelectPlugin = [this](KxMenuEvent& event)
			{
				const IGamePlugin* entry = IPluginManager::GetInstance()->FindPluginByName(event.GetItem()->GetItemLabelText());
				SelectItem(GetItemByEntry(entry), true);
				GetView()->SetFocus();
			};
			dependenciesMenu->Bind(KxEVT_MENU_SELECT, OnSelectPlugin);
			dependentMenu->Bind(KxEVT_MENU_SELECT, OnSelectPlugin);
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

	bool PluginViewModel::OnDragItems(KxDataViewEventDND& event)
	{
		if (CanDragDropNow())
		{
			if (const IGamePlugin* entry = GetDataEntry(GetRow(event.GetItem())))
			{
				KxDataViewItem::Vector selected;
				if (GetView()->GetSelections(selected) > 0)
				{
					std::unique_ptr<DragDropDataObjectT> dataObject;
					for (const auto& item: selected)
					{
						if (IGamePlugin* entry = GetDataEntry(GetRow(item)))
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
	bool PluginViewModel::OnDropItems(KxDataViewEventDND& event)
	{
		const IGamePlugin* entry = GetDataEntry(GetRow(event.GetItem()));
		if (entry && HasDragDropDataObject())
		{
			const IGamePlugin::RefVector& entriesToMove = GetDragDropDataObject()->GetEntries();

			// Move and refresh
			if (IPluginManager::GetInstance()->MovePlugins(entriesToMove, *entry))
			{
				ChangeNotify();
				RefreshItems();

				// Select moved items and event-select the first one
				for (IGamePlugin* entry: entriesToMove)
				{
					GetView()->Select(GetItemByEntry(entry));
				}
				SelectItem(GetItemByEntry(entriesToMove.front()));

				IEvent::MakeSend<PluginEvent>(Events::PluginsReordered, entriesToMove);
				return true;
			}
		}
		return false;
	}
	bool PluginViewModel::CanDragDropNow() const
	{
		if (KxDataViewColumn* column = GetView()->GetSortingColumn())
		{
			return column->GetID() == IDisplayModel::ColumnID::Priority && column->IsSortedAscending();
		}
		return true;
	}

	PluginViewModel::PluginViewModel()
		:m_Entries(IPluginManager::GetInstance()->GetPlugins())
	{
		SetDataViewFlags(KxDataViewCtrl::DefaultStyle|KxDV_MULTIPLE_SELECTION|KxDV_NO_TIMEOUT_EDIT|KxDV_VERT_RULES);
	}

	void PluginViewModel::ChangeNotify()
	{
		if (IGameProfile* profile = IGameInstance::GetActiveProfile())
		{
			profile->SyncWithCurrentState();
			profile->SaveConfig();
		}
		IPluginManager::GetInstance()->Save();
	}
	void PluginViewModel::SetDataVector()
	{
		KDataViewVectorListModel::SetDataVector();
	}
	void PluginViewModel::SetDataVector(IGamePlugin::Vector& array)
	{
		KDataViewVectorListModel::SetDataVector(&m_Entries);
	}
	IDisplayModel* PluginViewModel::GetDisplayModel() const
	{
		return m_DisplayModel.get();
	}

	void PluginViewModel::SetAllEnabled(bool value)
	{
		IPluginManager::GetInstance()->SetAllPluginsActive(value);
		GetView()->Refresh();
		ChangeNotify();
	}
	void PluginViewModel::UpdateUI()
	{
		GetView()->Refresh();
	}
	bool PluginViewModel::SetSearchMask(const wxString& mask)
	{
		return KAux::SetSearchMask(m_SearchMask, mask);
	}
}
