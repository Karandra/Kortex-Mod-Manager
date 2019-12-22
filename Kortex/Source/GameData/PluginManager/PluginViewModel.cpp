#include "stdafx.h"
#include <Kortex/PluginManager.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <Kortex/Application.hpp>
#include "GameMods/ModManager/Workspace.h"
#include "UI/TextEditDialog.h"
#include "Utility/MenuSeparator.h"
#include "Utility/UI.h"
#include <KxFramework/KxMenu.h>
#include <KxFramework/KxShell.h>
#include <KxFramework/KxComparator.h>

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
				if (plugin && Utility::UI::CheckSearchMask(m_SearchMask, plugin->GetName()))
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
		using Utility::MenuSeparatorBefore;

		KxDataViewItem item = event.GetItem();
		KxDataViewColumn* column = event.GetColumn();
		const IGamePlugin* plugin = GetDataEntry(GetRow(item));

		KxMenu menu;
		IPluginManager* manager = IPluginManager::GetInstance();
		Workspace* workspace = Workspace::GetInstance();

		// Base items
		{
			KxMenuItem* item = menu.AddItem(KTr("PluginManager.ActivateAll"));
			item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
			{
				SetAllEnabled(true);
			});
		}
		{
			KxMenuItem* item = menu.AddItem(KTr("PluginManager.DeactivateAll"));
			item->Bind(KxEVT_MENU_SELECT, [this](KxMenuEvent& event)
			{
				SetAllEnabled(false);
			});
		}

		const IBethesdaGamePlugin* bethesdaPlugin = nullptr;
		if (MenuSeparatorBefore sep(menu); plugin && plugin->QueryInterface(bethesdaPlugin))
		{
			// Description
			{
				wxString description = bethesdaPlugin->GetDescription();

				KxMenuItem* item = menu.AddItem(KTr("Generic.Description"));
				item->Enable(!description.IsEmpty());
				item->Bind(KxEVT_MENU_SELECT, [this, description](KxMenuEvent& event)
				{
					UI::TextEditDialog dialog(GetView());
					dialog.SetText(description);
					dialog.SetEditable(false);
					dialog.ShowPreview(true);
					dialog.ShowModal();
				});
			}

			// Plugin select event
			auto SelectPlugin = [this](KxMenuEvent& event)
			{
				const IGamePlugin* entry = IPluginManager::GetInstance()->FindPluginByName(event.GetItem()->GetItemLabelText());
				SelectItem(GetItemByEntry(entry), true);
				GetView()->SetFocus();
			};

			// Dependencies
			{
				KxStringVector dependenciesList = bethesdaPlugin->GetRequiredPlugins();
				KxMenu* dependenciesMenu = new KxMenu();
				KxMenuItem* dependenciesMenuItem = menu.Add(dependenciesMenu,
															wxString::Format(wxS("%s (%zu)"),
															KTr("PluginManager.PluginDependencies"),
															dependenciesList.size())
				);
				dependenciesMenuItem->Enable(!dependenciesList.empty());

				for (const wxString& name: dependenciesList)
				{
					KxMenuItem* item = dependenciesMenu->AddItem(name);
					item->SetBitmap(ImageProvider::GetBitmap(workspace->GetStatusImageForPlugin(manager->FindPluginByName(name))));
				}
				dependenciesMenu->Bind(KxEVT_MENU_SELECT, SelectPlugin);
			}

			// Dependent plugins
			{
				IGamePlugin::RefVector dependentList = plugin->GetDependentPlugins();
				KxMenu* dependentMenu = new KxMenu();
				KxMenuItem* dependentMenuItem = menu.Add(dependentMenu,
														 KxString::Format(wxS("%1 (%2)"),
														 KTr("PluginManager.DependentPlugins"),
														 dependentList.size())
				);
				dependentMenuItem->Enable(!dependentList.empty());

				for (const IGamePlugin* depEntry: dependentList)
				{
					KxMenuItem* item = dependentMenu->AddItem(depEntry->GetName());
					item->SetBitmap(ImageProvider::GetBitmap(workspace->GetStatusImageForPlugin(depEntry)));
				}
				dependentMenu->Bind(KxEVT_MENU_SELECT, SelectPlugin);
			}
		}

		// Workspace specific menus
		if (MenuSeparatorBefore sep(menu); true)
		{
			workspace->OnCreateViewContextMenu(menu, plugin);
			workspace->OnCreateSortingToolsMenu(menu, plugin);
			workspace->OnCreateImportExportMenu(menu, plugin);
		}

		// Misc options
		if (MenuSeparatorBefore sep(menu); true)
		{
			KxMenuItem* item = menu.AddItem(KTr("Generic.FileLocation"));
			item->Enable(plugin);
			item->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::FolderOpen));
			item->Bind(KxEVT_MENU_SELECT, [plugin](KxMenuEvent& event)
			{
				KxShell::OpenFolderAndSelectItem(plugin->GetFullPath());
			});
		}

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
		const IGamePlugin* anchorPlugin = GetDataEntry(GetRow(event.GetItem()));
		if (anchorPlugin && HasDragDropDataObject())
		{
			const IGamePlugin::RefVector& toMove = GetDragDropDataObject()->GetItems();

			// Move and refresh
			if (IPluginManager::GetInstance()->MovePlugins(toMove, *anchorPlugin))
			{
				ChangeNotify();
				RefreshItems();

				// Select moved items and event-select the first one
				for (IGamePlugin* plugin: toMove)
				{
					GetView()->Select(GetItemByEntry(plugin));
				}
				SelectItem(GetItemByEntry(toMove.front()));

				BroadcastProcessor::Get().ProcessEvent(PluginEvent::EvtReordered, toMove);
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
		:m_Items(IPluginManager::GetInstance()->GetPlugins())
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
		KxDataViewVectorListModelEx::SetDataVector();
	}
	void PluginViewModel::SetDataVector(IGamePlugin::Vector& array)
	{
		KxDataViewVectorListModelEx::SetDataVector(&m_Items);
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
		return Utility::UI::SetSearchMask(m_SearchMask, mask);
	}
}
