#pragma once
#include "stdafx.h"
#include "GameData/IGameSave.h"
#include <DataView2/DataView2.h>

namespace Kortex
{
	class ISaveManager;
}

namespace Kortex::SaveManager
{
	class Workspace;

	class DisplayModel: public KxDataView2::VirtualListModel
	{
		public:
			enum class ColumnID
			{
				Bitmap,
				Name,
				ModificationDate,
				Size,
			};

		private:
			BroadcastReciever m_BroadcastReciever;
			IGameSave::RefVector m_Saves;
			KBitmapSize m_BitmapSize;

			ISaveManager& m_Manager;
			Workspace* m_Workspace = nullptr;
			KxDataView2::Column* m_BitmapColumn = nullptr;

		private:
			KxDataView2::ToolTip GetToolTip(const  KxDataView2::Node& node, const  KxDataView2::Column& column) const override;
			wxAny GetEditorValue(const KxDataView2::Node& node, const KxDataView2::Column& column) const override;
			wxAny GetValue(const KxDataView2::Node& node, const KxDataView2::Column& column) const override;
			bool SetValue(KxDataView2::Node& node, KxDataView2::Column& column, const wxAny& value) override;
			bool IsEnabled(const KxDataView2::Node& node, const KxDataView2::Column& column) const override;
			bool GetAttributes(const KxDataView2::Node& node,
							   const KxDataView2::Column& column,
							   const KxDataView2::CellState& cellState,
							   KxDataView2::CellAttributes& attributes
			) const override;

			void Resort();
			bool Compare(const IGameSave& left, const IGameSave& right, const KxDataView2::Column& column) const;

			void OnSelectItem(KxDataView2::Event& event);
			void OnActivateItem(KxDataView2::Event& event);
			void OnContextMenu(KxDataView2::Event& event);
			void OnCacheHint(KxDataView2::Event& event);

			void OnHeaderSorted(KxDataView2::Event& event);
			void OnHeaderContextMenu(KxDataView2::Event& event);

			void OnFiltersChanged(BroadcastEvent& event);
			void OnVFSToggled(BroadcastEvent& event);

		public:
			DisplayModel();

		public:
			void CreateView(wxWindow* parent);
			void RefreshItems();
			void UpdateBitmapCellDimensions();

			const IGameSave& GetItem(const KxDataView2::Node& node) const
			{
				return *m_Saves[node.GetRow()];
			}
			IGameSave& GetItem(const KxDataView2::Node& node)
			{
				return *m_Saves[node.GetRow()];
			}
	};
}
