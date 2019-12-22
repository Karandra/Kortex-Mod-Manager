#pragma once
#include "stdafx.h"
#include "KxFramework/KxDataViewListModelEx.h"
#include "GameData/IPluginManager.h"
#include "BethesdaPluginManager.h"

namespace Kortex::PluginManager
{
	class IDisplayModel;
	class PluginViewDNDObject;

	class PluginViewModel:
		public KxDataViewVectorListModelEx<IGamePlugin::Vector, KxDataViewListModelEx>,
		public KxDataViewModelExDragDropEnabled<PluginViewDNDObject>
	{
		private:
			IGamePlugin::Vector& m_Items;
			std::unique_ptr<IDisplayModel> m_DisplayModel;
			wxString m_SearchMask;

		private:
			void OnInitControl() override;

			void GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const override;
			void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
			bool IsEditorEnabledByRow(size_t row, const KxDataViewColumn* column) const override;
			bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;
			bool GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const override;
			bool HasDefaultCompare() const override
			{
				return true;
			}
			bool CompareByRow(size_t row1, size_t row2, const KxDataViewColumn* column) const override;

			void OnSelectItem(KxDataViewEvent& event);
			void OnActivateItem(KxDataViewEvent& event);
			void OnContextMenu(KxDataViewEvent& event);

			KxDataViewCtrl* GetViewCtrl() const override
			{
				return GetView();
			}
			bool OnDragItems(KxDataViewEventDND& event) override;
			bool OnDropItems(KxDataViewEventDND& event) override;
			bool CanDragDropNow() const;

		public:
			PluginViewModel();

		public:
			void ChangeNotify();
			void SetDataVector();
			void SetDataVector(IGamePlugin::Vector& array);
			IDisplayModel* GetDisplayModel() const;

			const IGamePlugin* GetDataEntry(size_t index) const
			{
				return index < m_Items.size() ? &*m_Items[index] : nullptr;
			}
			IGamePlugin* GetDataEntry(size_t index)
			{
				return index < m_Items.size() ? &*m_Items[index] : nullptr;
			}
			KxDataViewItem GetItemByEntry(const IGamePlugin* entry) const
			{
				auto it = std::find_if(m_Items.begin(), m_Items.end(), [entry](const auto& v)
				{
					return v.get() == entry;
				});
				if (it != m_Items.end())
				{
					return GetItem(std::distance(m_Items.begin(), it));
				}
				return KxDataViewItem();
			}

			IGamePlugin* GetSelectedPlugin()
			{
				return GetDataEntry(GetSelectedPluginIndex());
			}
			int GetSelectedPluginIndex() const
			{
				size_t index = GetRow(GetView()->GetSelection());
				return index != (size_t)-1 ? index : (int)-1;
			}
			
			void SetAllEnabled(bool value);
			void UpdateUI();
			bool SetSearchMask(const wxString& mask);
	};
}

namespace Kortex::PluginManager
{
	class PluginViewDNDObject: public KxDataViewModelExDragDropData
	{
		private:
			IGamePlugin::RefVector m_Items;

		public:
			PluginViewDNDObject(size_t count = 0)
			{
				m_Items.reserve(count);
			}

		public:
			const IGamePlugin::RefVector& GetItems() const
			{
				return m_Items;
			}
			void AddEntry(IGamePlugin* entry)
			{
				m_Items.push_back(entry);
			}
	};
}
