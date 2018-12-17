#pragma once
#include "stdafx.h"
#include "KDataViewListModel.h"
#include "Store.h"

namespace Kortex::ModProvider
{
	class DisplayModel: public KxDataViewListModelExBase<KxDataViewModel>
	{
		protected:
			enum ColumnID
			{
				Name,
				Value
			};

		protected:
			Store& m_ProviderStore;
			bool m_IsModified = false;

		protected:
			void OnInitControl() override;

			void GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const override;
			bool IsContainer(const KxDataViewItem& item) const override;
			KxDataViewItem GetParent(const KxDataViewItem& item) const override;

			bool IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			void GetEditorValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			void GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const override;
			bool SetValue(const wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) override;

			void OnActivate(KxDataViewEvent& event);

		public:
			DisplayModel(Store& providerStore)
				:m_ProviderStore(providerStore)
			{
			}

		public:
			const Item* GetNode(const KxDataViewItem& item) const
			{
				return item.GetValuePtr<Item>();
			}
			Item* GetNode(const KxDataViewItem& item)
			{
				return item.GetValuePtr<Item>();
			}
			KxDataViewItem MakeItem(const Item& node)
			{
				return KxDataViewItem(&node);
			}

			bool IsModified() const
			{
				return m_IsModified;
			}
	};
}
