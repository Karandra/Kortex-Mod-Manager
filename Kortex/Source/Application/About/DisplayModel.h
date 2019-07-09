#pragma once
#include "stdafx.h"
#include "DisplayModelNode.h"
#include <KxFramework/DataView2/DataView2.h>
class KxHTMLWindow;

namespace Kortex::Application
{
	class AboutDialog;
}

namespace Kortex::Application::About
{
	class DisplayModel: public KxDataView2::VirtualListModel, public KxDataView2::TypeAliases
	{
		private:
			AboutDialog& m_Dialog;
			INode::Vector m_DataVector;

		private:
			wxAny GetValue(const Node& node, const Column& column) const override;
			bool GetAttributes(const Node& node, CellAttributes& attributes, const CellState& cellState, const Column& column) const override;

			void OnActivateItem(Event& event);

		private:
			template<class T, class... Args> INode& AddNode(Args&&... arg)
			{
				return *m_DataVector.emplace_back(std::make_unique<T>(std::forward<Args>(arg)...));
			}
			template<class... Args> INode& AddSoftwareNode(Args&&... arg)
			{
				return AddNode<SoftwareNode>(std::forward<Args>(arg)...);
			}
			template<class... Args> INode& AddResourceNode(Args&&... arg)
			{
				return AddNode<ResourceNode>(std::forward<Args>(arg)...);
			}

			const INode& GetItem(const Node& node) const
			{
				return *m_DataVector[node.GetRow()];
			}

		public:
			DisplayModel(AboutDialog& dialog);

		public:
			void RefreshItems();
			void CreateView(wxWindow* parent);
	};
}
