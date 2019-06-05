#pragma once
#include "stdafx.h"
#include "Application/Resources/ImageResourceID.h"
#include <KxFramework/DataView2/DataView2.h>
#include "PackageProject/KPackageProject.h"

namespace Kortex::InstallWizard
{
	class WizardDialog;
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

namespace Kortex::InstallWizard
{
	class InfoDisplayModel: public KxDataView2::VirtualListModel
	{
		private:
			WizardDialog& m_InstallWizard;
			KPackageProject& m_PackageConfig;
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
			InfoDisplayModel(WizardDialog& installWizard, KPackageProject& packageProject)
				:m_InstallWizard(installWizard), m_PackageConfig(packageProject)
			{
			}

		public:
			void CreateView(wxWindow* parent);
			void AddItem(const KLabeledValue& value, const ResourceID& image = {}, InfoKind type = InfoKind::None);
	};
}
