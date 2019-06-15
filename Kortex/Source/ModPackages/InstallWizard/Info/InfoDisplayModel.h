#pragma once
#include "stdafx.h"
#include "InfoPage.h"
#include <KxFramework/DataView2/DataView2.h>

namespace Kortex::InstallWizard::InfoPageNS
{
	enum class InfoKind
	{
		None = 0,
		ID,
		Name,
		ModSource,
		Tags,
	};

	class Item
	{
		friend class InfoDisplayModel;

		private:
			KLabeledValue Value;
			ResourceID IconID;
			InfoKind Type = InfoKind::None;

		public:
			Item(const KLabeledValue& value)
				:Value(value)
			{
			}
	};
}

namespace Kortex::InstallWizard::InfoPageNS
{
	class InfoDisplayModel: public KxDataView2::VirtualListModel
	{
		private:
			InfoPage& m_Page;
			std::vector<Item> m_Items;

		private:
			wxAny GetValue(const KxDataView2::Node& node, const KxDataView2::Column& column) const override;
			wxAny GetEditorValue(const KxDataView2::Node& node, const KxDataView2::Column& column) const override;
			bool SetValue(KxDataView2::Node& node, const wxAny& value, KxDataView2::Column& column) override;
			bool IsEnabled(const KxDataView2::Node& node, const KxDataView2::Column& column) const override;
			bool GetAttributes(const KxDataView2::Node& node, KxDataView2::CellAttributes& attributes, const KxDataView2::CellState& cellState, const KxDataView2::Column& column) const override;

			bool CheckModID(const wxString& id);
			void OnActivateItem(KxDataView2::Event& event);

		public:
			InfoDisplayModel(InfoPage& page, size_t initialCount = 0)
				:m_Page(page)
			{
				m_Items.reserve(initialCount);
			}

		public:
			void CreateView(wxWindow* parent);
			void AddItem(const KLabeledValue& value, const ResourceID& image = {}, InfoKind type = InfoKind::None);
	};
}
