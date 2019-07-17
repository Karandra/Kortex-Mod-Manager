#include "stdafx.h"
#include "DisplayModel.h"
#include "Workspace.h"
#include "UI/KWorkspaceController.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameConfig.hpp>
#include <Kortex/Utility.hpp>
#include <KxFramework/DataView2/DataView2.h>

namespace Kortex::GameConfig
{
	bool DisplayModel::OnAskRefreshView()
	{
		Workspace* workspace = Workspace::GetInstance();
		if (workspace)
		{
			if (KWorkspaceController* controller = workspace->GetWorkspaceController())
			{
				return controller->AskForSave();
			}
		}
		return true;
	}
	void DisplayModel::ExpandAllCategories()
	{
		std::function<void(KxDataView2::Node&)> Expand = [this, &Expand](KxDataView2::Node& parentNode)
		{
			for (KxDataView2::Node* node: parentNode.GetChildren())
			{
				if (node->QueryInterface<CategoryItem>())
				{
					node->Expand();
				}
				if (node->HasChildren())
				{
					Expand(*node);
				}
			}
		};
		Expand(GetView()->GetRootNode());
	}

	void DisplayModel::OnActivate(KxDataView2::Event& event)
	{
		using namespace KxDataView2;

		if (event.GetColumn())
		{
			IViewItem* viewItem = nullptr;
			if (Node* node = event.GetNode(); node && node->QueryInterface(viewItem))
			{
				if (event.GetEventType() == EvtITEM_ACTIVATED)
				{
					viewItem->OnActivate(*event.GetColumn());
				}
				else if (event.GetEventType() == EvtITEM_SELECTED)
				{
					viewItem->OnSelect(*event.GetColumn());
				}
			}
		}
	}
	void DisplayModel::OnContextMenu(KxDataView2::Event& event)
	{
		Item* item = event.GetNode() ? event.GetNode()->QueryInterface<Item>() : nullptr;
		KxDataView2::Column* column = event.GetColumn();

		KxMenu menu;
		{
			KxMenuItem* menuItem = menu.Add(new KxMenuItem(KxID_EDIT, m_Translator.GetString("ConfigManager.Menu.EditValue")));
			menuItem->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::PencilSmall));
			menuItem->SetDefault();
			menuItem->Enable(item);
		}
		{
			KxMenuItem* menuItem = menu.Add(new KxMenuItem(KxID_UNDO, m_Translator.GetString("ConfigManager.Menu.DiscardChange")));
			menuItem->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::CrossWhite));
			menuItem->Enable(item && item->HasChanges());
		}
		{
			KxMenuItem* menuItem = menu.Add(new KxMenuItem(KxID_REFRESH, m_Translator.GetString(KxID_REFRESH)));
			menuItem->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::ArrowCircleDouble));
		}

		menu.AddSeparator();
		{
			KxMenuItem* menuItem = menu.Add(new KxMenuItem(KxID_DELETE, m_Translator.GetString("ConfigManager.Menu.RemoveValue")));
			menuItem->SetBitmap(ImageProvider::GetBitmap(ImageResourceID::MinusSmall));
			menuItem->Enable(item);
		}

		switch (menu.Show(GetView()))
		{
			case KxID_EDIT:
			{
				item->OnActivate(*GetView()->GetColumnByID(ColumnID::Value));
				break;
			}
			case KxID_UNDO:
			{
				item->DiscardChanges();
				break;
			}
			case KxID_REFRESH:
			{
				if (OnAskRefreshView())
				{
					m_Manager.Load();
					LoadView();
				}
				break;
			}

			case KxID_DELETE:
			{
				item->DeleteValue();
				break;
			}
		};
	}

	DisplayModel::DisplayModel(IConfigManager& manager)
		:m_Manager(manager), m_Translator(manager.GetTranslator())
	{
		m_Manager.OnCreateDisplayModel(*this);
	}
	DisplayModel::~DisplayModel()
	{
		m_Manager.OnDestroyDisplayModel(*this);
		ClearView();
	}
	
	void DisplayModel::CreateView(wxWindow* parent, wxSizer* sizer)
	{
		using namespace KxDataView2;

		KxDataView2::View* view = new View(parent, KxID_NONE, CtrlStyle::VerticalRules|CtrlStyle::CellFocus|CtrlStyle::FitLastColumn);
		view->SetModel(this);
		if (sizer)
		{
			sizer->Add(view, 1, wxEXPAND);
		}

		ColumnStyle columnStyle = ColumnStyle::Move|ColumnStyle::Size|ColumnStyle::Sort;
		view->AppendColumn<BitmapTextRenderer>(m_Translator.GetString("ConfigManager.View.Path"), ColumnID::Path, {}, columnStyle);
		view->AppendColumn<TextRenderer>(m_Translator.GetString("ConfigManager.View.Name"), ColumnID::Name, {}, columnStyle);
		view->AppendColumn<TextRenderer>(m_Translator.GetString("ConfigManager.View.Type"), ColumnID::Type, {}, columnStyle);
		view->AppendColumn<BitmapTextToggleRenderer>(m_Translator.GetString("ConfigManager.View.Value"), ColumnID::Value, {}, columnStyle);

		view->Bind(EvtITEM_ACTIVATED, &DisplayModel::OnActivate, this);
		view->Bind(EvtITEM_SELECTED, &DisplayModel::OnActivate, this);
		view->Bind(EvtITEM_CONTEXT_MENU, &DisplayModel::OnContextMenu, this);
		view->Bind(EvtCOLUMN_HEADER_RCLICK, [this, view](Event& event)
		{
			if (!m_DisableColumnsMenu)
			{
				KxMenu menu;
				view->CreateColumnSelectionMenu(menu);
				view->OnColumnSelectionMenu(menu);
			}
			else
			{
				event.Skip();
			}
		});
	}
	void DisplayModel::ClearView()
	{
		if (KxDataView2::View* view = GetView())
		{
			view->GetRootNode().DetachAllChildren();
		}
		m_Categories.clear();
	}
	void DisplayModel::LoadView()
	{
		ClearView();
		m_Manager.ForEachItem([this](Item& item)
		{
			KxDataView2::Node* parent = &GetView()->GetRootNode();
			auto it = m_Categories.find(item.GetCategory());
			if (it != m_Categories.end())
			{
				parent = &it->second;
			}
			else
			{
				// Build category tree for this item
				wxString categoryPath;
				Utility::String::SplitBySeparator(item.GetCategory(), wxS('/'), [this, &parent, &categoryPath](const auto& category)
				{
					// Add next part
					if (categoryPath.IsEmpty())
					{
						categoryPath = Utility::String::FromWxStringView(category);
					}
					else
					{
						categoryPath = Utility::String::ConcatWithSeparator(wxS('/'), categoryPath, Utility::String::FromWxStringView(category));
					}

					// See if that branch already exist
					auto it = m_Categories.find(categoryPath);
					if (it != m_Categories.end())
					{
						parent = &it->second;
					}
					else
					{
						// No branch, create one
						auto[it, inserted] = m_Categories.insert_or_assign(categoryPath, categoryPath);

						// Attach to view nodes
						CategoryItem& categoryItem = it->second;
						parent->AttachChild(categoryItem, 0);
						parent = &categoryItem;
					}
					return true;
				});
			}

			parent->AttachChild(item, parent->GetChildrenCount());
			item.OnAttachToView();
		});
		GetView()->ItemsChanged();

		if (m_ExpandBranches)
		{
			ExpandAllCategories();
		}
	}
	void DisplayModel::RefreshView()
	{
		if (KxDataView2::View* view = GetView())
		{
			view->Refresh();
		}
	}
}
