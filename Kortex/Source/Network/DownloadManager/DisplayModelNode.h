#pragma once
#include "stdafx.h"
#include <DataView2/DataView2.h>

namespace Kortex
{
	class DownloadItem;
}

namespace Kortex::DownloadManager
{
	class DisplayModel;

	class DisplayModelNode: public KxDataView2::Node
	{
		friend class DisplayModel;

		public:
			enum class ColumnID
			{
				Name,
				Version,
				Size,
				Game,
				ModNetwork,
				Date,
				Status,
			};

		private:
			DownloadItem& m_Item;
			KxDataView2::Column* m_SizeColumn = nullptr;

		private:
			wxAny GetValue(const KxDataView2::Column& column) const override;
			bool SetValue(KxDataView2::Column& column, const wxAny& value) override;
			KxDataView2::ToolTip GetToolTip(const KxDataView2::Column& column) const override;

			bool IsEnabled(const KxDataView2::Column& column) const override;
			bool Compare(const Node& otherNode, const KxDataView2::Column& column) const override;

			wxBitmap GetStateBitmap() const;

		protected:
			void OnAttachNode()
			{
				m_SizeColumn = GetView()->GetColumnByID(ColumnID::Size);
			}

		public:
			DisplayModelNode(DownloadItem& item)
				:m_Item(item)
			{
			}
	};
}
