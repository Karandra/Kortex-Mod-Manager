#include "stdafx.h"
#include "DisplayModel.h"
#include "Utility/KAux.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameConfig.hpp>
#include <Kortex/Utility.hpp>
#include <KxFramework/DataView2/DataView2.h>

namespace Kortex::GameConfig
{
	void DisplayModel::OnDeleteNode(KxDataView2::Node* node)
	{
		// We own CategoryItem nodes and config manager owns everything else,
		// so don't delete anything here.
	}
	void DisplayModel::OnDetachRootNode(KxDataView2::RootNode& node)
	{
		// Root node doesn't own anything
		node.DetachAllChildren();
	}

	void DisplayModel::OnActivate(KxDataView2::Event& event)
	{
		using namespace KxDataView2;

		if (event.GetColumn())
		{
			Item* item = nullptr;
			if (Node* node = event.GetNode(); node && node->QueryInterface(item))
			{
				if (event.GetEventType() == KxEVT_DATAVIEW_ITEM_ACTIVATED)
				{
					item->OnActivate(*event.GetColumn());
				}
				else if (event.GetEventType() == KxEVT_DATAVIEW_ITEM_SELECTED)
				{
					item->OnSelect(*event.GetColumn());
				}
			}
		}
	}

	DisplayModel::DisplayModel()
		:m_Manager(*IGameConfigManager::GetInstance()), m_Translator(m_Manager.GetTranslator())
	{
	}
	DisplayModel::~DisplayModel()
	{
		ClearView();
	}
	
	void DisplayModel::CreateView(wxWindow* parent, wxSizer* sizer)
	{
		using namespace KxDataView2;

		KxDataView2::View* view = new View(parent, KxID_NONE, CtrlStyle::VerticalRules|CtrlStyle::CellFocus);
		view->SetModel(this);
		sizer->Add(view, 1, wxEXPAND);

		ColumnStyle columnStyle = ColumnStyle::Move|ColumnStyle::Size|ColumnStyle::Sort;
		view->AppendColumn<BitmapTextRenderer>(m_Translator.GetString("ConfigManager.View.Path"), ColumnID::Path, {}, columnStyle);
		view->AppendColumn<TextRenderer>(m_Translator.GetString("ConfigManager.View.Name"), ColumnID::Name, {}, columnStyle);
		view->AppendColumn<TextRenderer>(m_Translator.GetString("ConfigManager.View.Type"), ColumnID::Type, {}, columnStyle);
		view->AppendColumn<BitmapTextToggleRenderer>(m_Translator.GetString("ConfigManager.View.Value"), ColumnID::Value, {}, columnStyle);

		view->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &DisplayModel::OnActivate, this);
		view->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &DisplayModel::OnActivate, this);
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
						parent->AttachChild(&it->second, 0);
						parent = &it->second;
					}
					return true;
				});
			}

			parent->AttachChild(&item, parent->GetChildrenCount());
			item.OnAttachToView();
		});
		GetView()->ItemsChanged();
	}
	void DisplayModel::RefreshView()
	{
		if (KxDataView2::View* view = GetView())
		{
			view->Refresh();
		}
	}
}
