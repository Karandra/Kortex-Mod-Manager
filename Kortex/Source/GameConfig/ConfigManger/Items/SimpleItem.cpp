#include "stdafx.h"
#include "SimpleItem.h"
#include "Utility/KAux.h"

namespace Kortex::GameConfig
{
	bool SimpleItem::Create(const KxXMLNode& itemNode)
	{
		return IsOK();
	}

	void SimpleItem::Clear()
	{
		m_Value.MakeNull();
	}
	void SimpleItem::Read(const ISource& source)
	{
		source.ReadValue(*this, m_Value);
	}
	void SimpleItem::Write(ISource& source) const
	{
		source.WriteValue(*this, m_Value);
	}

	SimpleItem::SimpleItem(ItemGroup& group, const KxXMLNode& itemNode)
		:IImplementation<Item>(group, itemNode)
	{
	}
	SimpleItem::SimpleItem(ItemGroup& group, bool isUnknown)
		:IImplementation<Item>(group), m_IsUnknown(isUnknown)
	{
	}

	wxAny SimpleItem::GetValue(const Column& column) const
	{
		switch (column.GetID<ColumnID>())
		{
			case ColumnID::Path:
			{
				return GetFullPath();
			}
			case ColumnID::Name:
			{
				return GetLabel();
			}
			case ColumnID::Type:
			{
				return GetTypeID().ToString();
			}
			case ColumnID::Value:
			{
				const ItemValue& value = GetValue();
				wxString serializedValue = value.Serialize(*this);

				if (serializedValue.IsEmpty())
				{
					if (value.GetType().IsString())
					{
						return KAux::MakeBracketedLabel(GetManager().GetTranslator().GetString(wxS("ConfigManager.View.EmptyStringValue")));
					}
					else
					{
						return KAux::MakeNoneLabel();
					}
				}
				else
				{
					return serializedValue;
				}
			}
		}
		return {};
	}
	KxDataView2::Renderer& SimpleItem::GetRenderer(const Column& column) const
	{
		return m_Renderer;
	}
	KxDataView2::Editor* SimpleItem::GetEditor(const Column& column) const
	{
		return nullptr;
	}
	bool SimpleItem::GetAttributes(CellAttributes& attributes, const CellState& cellState, const Column& column) const
	{
		if (m_IsUnknown)
		{
			attributes.SetEnabled(false);
			return true;
		}
		return false;
	}
}
