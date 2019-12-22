#pragma once
#include "stdafx.h"
#include <KxFramework/DataView2/DataView2.h>
#include "BaseNotification.h"
#include "Utility/BitmapSize.h"

namespace Kortex::Notifications
{
	class DisplayModel: public KxDataView2::VirtualListModel, public KxDataView2::TypeAliases
	{
		private:
			Utility::BitmapSize m_BitmapSize;
			INotification::Vector& m_Notifications;

		private:
			wxAny GetValue(const Node& node, const Column& column) const override;
			ToolTip GetToolTip(const Node& node, const Column& column) const override;
			bool GetAttributes(const Node& node, const Column& column, const CellState& cellState, CellAttributes& attributes) const override;

			void OnSelectItem(Event& event);
			void OnActivateItem(Event& event);

			const INotification& GetItem(const Node& node) const
			{
				return *m_Notifications[node.GetRow()];
			}
			INotification& GetItem(Node& node)
			{
				return *m_Notifications[node.GetRow()];
			}
			wxString FormatText(const INotification& notification) const;

		public:
			DisplayModel();

		public:
			void CreateView(wxWindow* parent);
			void OnShowWindow();
			void RefreshItems();
	};
}
