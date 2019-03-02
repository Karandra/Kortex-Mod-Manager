#include "stdafx.h"
#include "CategoryItem.h"
#include "GameConfig/IGameConfigManager.h"
#include "GameConfig/ConfigManger/Common.h"
#include "Utility/KAux.h"
#include <KxFramework/KxComparator.h>

namespace Kortex::GameConfig
{
	wxString CategoryItem::TranslateCategoryName() const
	{
		const IGameConfigManager* manager = IGameConfigManager::GetInstance();
		if (!m_CategoryPath.IsEmpty())
		{
			wxString name = m_CategoryPath.AfterLast(wxS('/'));

			wxString label = manager->TranslateItemLabel(name, wxS("Category"));
			if (label.IsEmpty())
			{
				label = manager->TranslateItemLabel(name, wxS("Category.ENB"));
			}
			return label;
		}
		return manager->GetTranslator().GetString(wxS("ConfigManager.Categories.None"));
	}

	CategoryItem::CategoryItem(const wxString& categoryPath)
		:m_CategoryPath(categoryPath), m_CategoryName(TranslateCategoryName())
	{
	}
	CategoryItem::CategoryItem(const wxString& categoryPath, const wxString& categoryName)
		:m_CategoryPath(categoryPath), m_CategoryName(categoryName)
	{
	}
	CategoryItem::~CategoryItem()
	{
		DetachAllChildren();
	}

	wxString CategoryItem::GetViewString(ColumnID id) const
	{
		if (id == ColumnID::Path)
		{
			return m_CategoryName;
		}
		if (id == ColumnID::Name)
		{
			KxDataView2::Column* column = GetView()->GetColumnByID(ColumnID::Path);
			if (column && !column->IsVisible())
			{
				return m_CategoryName;
			}
		}
		return wxString();
	}
	void CategoryItem::OnActivate(KxDataView2::Column& column)
	{
		if (column.GetID<ColumnID>() == ColumnID::Path)
		{
			ToggleExpanded();
		}
	}

	wxAny CategoryItem::GetValue(const KxDataView2::Column& column) const
	{
		return GetViewString(column.GetID<ColumnID>());
	}
	KxDataView2::Renderer& CategoryItem::GetRenderer(const KxDataView2::Column& column) const
	{
		return column.GetRenderer();
	}
	KxDataView2::Editor* CategoryItem::GetEditor(const KxDataView2::Column& column) const
	{
		return nullptr;
	}
	bool CategoryItem::GetAttributes(KxDataView2::CellAttributes& attributes, const KxDataView2::CellState& cellState, const KxDataView2::Column& column) const
	{
		return false;
	}
}
