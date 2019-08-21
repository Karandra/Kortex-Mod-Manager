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

			ISaveManager& m_Manager;
			Workspace* m_Workspace = nullptr;

			KxDataView2::Column* m_BitmapColumn = nullptr;
			KBitmapSize m_DefaultBitmapSize;
			KBitmapSize m_NativeBitmapSize;
			KBitmapSize m_MinBitmapSize;
			KBitmapSize m_BitmapSize;
			double m_BitmapRatio = 0;
			int m_MaxWidth = 0;

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
		

			bool UpdateBitmapSize(int width = -1);
			void UpdateBitmapCellDimensions();
			void UpdateNativeBitmapSize();

			void OnSelectItem(KxDataView2::Event& event);
			void OnActivateItem(KxDataView2::Event& event);
			void OnContextMenu(KxDataView2::Event& event);
			void OnCacheHint(KxDataView2::Event& event);

			void OnHeaderSorted(KxDataView2::Event& event);
			void OnHeaderResized(KxDataView2::Event& event);
			void OnHeaderMenuItem(KxDataView2::Event& event);

			void OnFiltersChanged(BroadcastEvent& event);
			void OnVFSToggled(BroadcastEvent& event);

		public:
			DisplayModel();

		public:
			void CreateView(wxWindow* parent);
			void RefreshItems();

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
