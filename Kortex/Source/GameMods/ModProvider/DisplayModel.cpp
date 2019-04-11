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
			children.reserve(m_ModSourceStore.GetSize());
			m_ModSourceStore.Visit([&children](ModSourceItem& item)
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
		const ModSourceItem* node = GetNode(item);
		if (node)
		{
			if (node->HasModSource())
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
		const ModSourceItem* node = GetNode(item);
		if (node)
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					value = node->GetName();
					break;
				}
				case ColumnID::Value:
				{
					if (node->HasModSource())
					{
						value = node->GetModInfo().ToString();
					}
					else
					{
						value = node->GetURL();
					}
					break;
				}
			};
		}
	}
	void DisplayModel::GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
	{
		const ModSourceItem* node = GetNode(item);
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				INetworkModSource* modSource = nullptr;
				value = KxDataViewBitmapTextValue(node->GetName(), KGetBitmap(node->TryGetModSource(modSource) ? modSource->GetIcon() : INetworkModSource::GetGenericIcon()));
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
		ModSourceItem* node = GetNode(item);

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
				if (node->HasModSource())
				{
					NetworkModInfo modInfo;
					if (modInfo.FromString(newValue))
					{
						node->SetModInfo(modInfo);
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
		const ModSourceItem* node = GetNode(event.GetItem());
		if (node)
		{
			// Allow edit both name and value for unknown sources (with URL),
			// and restrict editing to modID only for known sources.
			KxDataViewColumn* column = node->HasModSource() ? GetView()->GetColumnByID(ColumnID::Value) : event.GetColumn();
			if (column)
			{
				GetView()->EditItem(event.GetItem(), column);
			}
		}
	}
	void DisplayModel::ApplyChanges()
	{
		m_ModSourceStore.RemoveIf([](ModSourceItem& item)
		{
			return !item.IsOK();
		});
	}
}
