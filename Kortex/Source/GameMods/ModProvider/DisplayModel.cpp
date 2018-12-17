#include "stdafx.h"
#include "DisplayModel.h"

namespace Kortex::ModProvider
{
	void DisplayModel::OnInitControl()
	{
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &DisplayModel::OnActivate, this);

		// Columns
		GetView()->AppendColumn<KxDataViewBitmapTextRenderer, KxDataViewTextEditor>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_EDITABLE, 150);
		GetView()->AppendColumn<KxDataViewTextRenderer, KxDataViewTextEditor>(KTr("Generic.Value"), ColumnID::Value, KxDATAVIEW_CELL_EDITABLE);
	}

	void DisplayModel::GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const
	{
		if (item.IsTreeRootItem())
		{
			children.reserve(m_ProviderStore.GetSize());
			m_ProviderStore.Visit([&children](Item& item)
			{
				children.push_back(&item);
				return true;
			});
		}
	}
	bool DisplayModel::IsContainer(const KxDataViewItem& item) const
	{
		return item.IsTreeRootItem();
	}
	KxDataViewItem DisplayModel::GetParent(const KxDataViewItem& item) const
	{
		return KxDataViewItem();
	}

	bool DisplayModel::IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		const Item* node = GetNode(item);
		if (node)
		{
			if (node->HasProvider())
			{
				return column->GetID() == ColumnID::Value;
			}
			else
			{
				switch (column->GetID())
				{
					case ColumnID::Name:
					case ColumnID::Value:
					{
						return true;
					}
				};
			}
		}
		return false;
	}
	void DisplayModel::GetEditorValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		const Item* node = GetNode(item);
		if (node)
		{
			switch (column->GetID())
			{
				case ColumnID::Value:
				{
					Network::ModID id = Network::InvalidModID;
					value = node->TryGetModID(id) ? std::to_wstring(id) : node->GetURL();
					return;
				}
			};
			GetValue(value, item, column);
		}
	}
	void DisplayModel::GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		const Item* node = GetNode(item);
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				INetworkProvider* provider = nullptr;
				value = KxDataViewBitmapTextValue(node->GetName(), KGetBitmap(node->TryGetProvider(provider) ? provider->GetIcon() : INetworkProvider::GetGenericIcon()));
				break;
			}
			case ColumnID::Value:
			{
				value = node->GetURL();
				break;
			}
		};
	}
	bool DisplayModel::SetValue(const wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column)
	{
		Item* node = GetNode(item);

		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				wxString name = data.As<wxString>();
				if (name != node->GetName())
				{
					node->SetName(name);
					m_IsModified = true;
					return true;
				}
				break;
			}
			case ColumnID::Value:
			{
				wxString newValue = data.As<wxString>();
				if (node->HasProvider())
				{
					Network::ModID modID = Network::InvalidModID;
					if (data.GetAs(&modID))
					{
						node->SetModID(modID);
						m_IsModified = true;
						return true;
					}
				}
				else
				{
					if (newValue != node->GetURL())
					{
						node->SetURL(newValue);
						m_IsModified = true;
						return true;
					}
				}
			}
		};
		return false;
	}

	void DisplayModel::OnActivate(KxDataViewEvent& event)
	{
		const Item* node = GetNode(event.GetItem());
		if (node)
		{
			KxDataViewColumn* column = node->HasProvider() ? event.GetColumn() : GetView()->GetColumnByID(ColumnID::Value);
			if (column)
			{
				GetView()->EditItem(event.GetItem(), column);
			}
		}
	}
}
