#pragma once
#include "stdafx.h"
#include "RequirementsPage.h"
#include <KxFramework/DataView2/DataView2.h>

namespace Kortex::InstallWizard::RequirementsPageNS
{
	class DisplayModel: public KxDataView2::VirtualListModel, public KxDataView2::TypeAliases
	{
		private:
			RequirementsPage& m_Page;
			PackageProject::KPPRRequirementEntry::RefVector m_Items;

		private:
			wxAny GetValue(const Node& node, const Column& column) const override;
			ToolTip GetToolTip(const Node& node, const Column& column) const override;
			bool IsEnabled(const Node& node, const Column& column) const override;

			wxBitmap GetIconByState(PackageProject::KPPReqState state) const;
			wxBitmap GetIconByState(bool state) const
			{
				return GetIconByState(state ? PackageProject::KPPReqState::True : PackageProject::KPPReqState::False);
			}

		public:
			DisplayModel(RequirementsPage& page)
				:m_Page(page)
			{
			}

		public:
			void CreateView(wxWindow* parent, bool noBorder = false);
			void ShowGroups(const KxStringVector& groupIDs);
			void ClearDisplay();

			const PackageProject::KPPRRequirementEntry& GetItem(const Node& node) const
			{
				return *m_Items[node.GetRow()];
			}
			PackageProject::KPPRRequirementEntry& GetItem(Node& node)
			{
				return *m_Items[node.GetRow()];
			}
	};
}
