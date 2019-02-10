#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/IViewItem.h"
#include <KxFramework/DataView2/DataView2.h>

namespace Kortex::GameConfig
{
	class CategoryItem: public RTTI::IExtendInterface<CategoryItem, IViewItem, KxDataView2::Node>
	{
		private:
			wxString m_CategoryPath;
			wxString m_CategoryName;

		private:
			wxString TranslateCategoryName() const;

		public:
			CategoryItem(const wxString& categoryPath);
			CategoryItem(const wxString& categoryPath, const wxString& categoryName);
			~CategoryItem();

		public:
			wxString GetCategoryPath() const
			{
				return m_CategoryPath;
			}
			wxString GetCategoryName() const
			{
				return m_CategoryName;
			}

			wxString GetStringRepresentation(ColumnID id) const override;

		public:
			wxAny GetValue(const KxDataView2::Column& column) const override;
			KxDataView2::Renderer& GetRenderer(const KxDataView2::Column& column) const override;
			KxDataView2::Editor* GetEditor(const KxDataView2::Column& column) const override;
			bool GetAttributes(KxDataView2::CellAttributes& attributes, const KxDataView2::CellState& cellState, const KxDataView2::Column& column) const override;
	};
}
