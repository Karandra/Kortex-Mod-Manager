#pragma once
#include "stdafx.h"
#include "Utility/KDataViewListModel.h"
#include "DisplayModelNode.h"
class KxHTMLWindow;

namespace Kortex::Application
{
	class AboutDialog;
}

namespace Kortex::Application::About
{
	class DisplayModel: public KxDataViewVectorListModelEx<INode::Vector, KxDataViewListModelEx>
	{
		private:
			AboutDialog& m_Dialog;
			INode::Vector m_DataVector;

		private:
			void OnInitControl() override;

			void GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
			bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
			bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;
			bool GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attribute, KxDataViewCellState cellState) const override;

			void OnActivateItem(KxDataViewEvent& event);

		private:
			template<class... Args> INode& AddSoftwareNode(Args&&... arg)
			{
				return *m_DataVector.emplace_back(std::make_unique<SoftwareNode>(std::forward<Args>(arg)...));
			}
			template<class... Args> INode& AddResourceNode(Args&&... arg)
			{
				return *m_DataVector.emplace_back(std::make_unique<ResourceNode>(std::forward<Args>(arg)...));
			}

		public:
			DisplayModel(AboutDialog& dialog);

		public:
			const INode* GetDataEntry(size_t row) const
			{
				if (row < GetItemCount())
				{
					return m_DataVector[row].get();
				}
				return nullptr;
			}
			void RefreshItems() override;
	};
}
