#pragma once
#include "stdafx.h"
#include "GameConfig/ConfigManger/IViewItem.h"
#include <KxFramework/DataView2/DataView2.h>

namespace Kortex::GameConfig
{
	class CategoryItem: public KxRTTI::ExtendInterface<CategoryItem, IViewItem, KxDataView2::Node>
	{
		KxDecalreIID(CategoryItem, {0x1b227221, 0xe3ff, 0x4196, {0x96, 0x14, 0x94, 0xbc, 0x97, 0x71, 0xa0, 0x3a}});

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

		public:
			wxString GetViewString(ColumnID id) const override;
			void OnActivate(KxDataView2::Column& column) override;

			wxAny GetValue(const KxDataView2::Column& column) const override;
			KxDataView2::Renderer& GetRenderer(const KxDataView2::Column& column) const override;
			KxDataView2::Editor* GetEditor(const KxDataView2::Column& column) const override;
			
			bool GetAttributes(const KxDataView2::Column& column, const KxDataView2::CellState& cellState, KxDataView2::CellAttributes& attributes) const override;
	};
}
