#include "stdafx.h"
#include "StructItem.h"
#include <KxFramework/DataView2/DataView2.h>
#include <KxFramework/KxStringUtility.h>

namespace Kortex::GameConfig
{
	bool StructItem::Create(const KxXMLNode& itemNode)
	{
		const KxXMLNode subItemsNode = itemNode.GetFirstChildElement(wxS("SubItems"));
		m_SubItems.reserve(subItemsNode.GetChildrenCount());

		// Load sub-items
		for (KxXMLNode node = subItemsNode.GetFirstChildElement(wxS("Item")); node.IsOK(); node = node.GetNextSiblingElement(wxS("Item")))
		{
			TypeID type;
			if (type.FromString(node.GetAttribute(wxS("Type"))) && type.IsScalarType())
			{
				StructSubItem& subItem = m_SubItems.emplace_back(*this, node);
				if (!subItem.Create(node))
				{
					m_SubItems.pop_back();
				}
			}
		}
		return IsOK();
	}

	void StructItem::Clear()
	{
		for (StructSubItem& subItem: m_SubItems)
		{
			subItem.Clear();
		}
	}
	void StructItem::Read(const ISource& source)
	{
		switch (m_SerializationMode.GetValue())
		{
			case StructSerializationModeID::ElementWise:
			{
				for (StructSubItem& subItem: m_SubItems)
				{
					subItem.Read(source);
				}
				break;
			}
			case StructSerializationModeID::AsString:
			{
				break;
			}
		};
	}
	void StructItem::Write(ISource& source) const
	{
		switch (m_SerializationMode.GetValue())
		{
			case StructSerializationModeID::ElementWise:
			{
				for (const StructSubItem& subItem: m_SubItems)
				{
					subItem.Write(source);
				}
				break;
			}
			case StructSerializationModeID::AsString:
			{
				break;
			}
		};
	}
	void StructItem::ChangeNotify()
	{
		for (StructSubItem& subItem: m_SubItems)
		{
			subItem.ResetCache();
		}
		m_CachedViewValue.reset();
		Item::ChangeNotify();
	}

	std::unique_ptr<KxDataView2::Editor> StructItem::CreateEditor() const
	{
		return nullptr;
	}

	StructItem::StructItem(ItemGroup& group, const KxXMLNode& itemNode)
		:IExtendInterface(group, itemNode)
	{
	}
	StructItem::StructItem(ItemGroup& group, bool isUnknown)
		:IExtendInterface(group)
	{
	}

	void StructItem::OnAttachToView()
	{
		for (StructSubItem& subItem: m_SubItems)
		{
			AttachChild(&subItem, GetChildrenCount());
		}
	}
	wxString StructItem::GetStringRepresentation(ColumnID id) const
	{
		if (id == ColumnID::Type)
		{
			if (!m_CachedViewType)
			{
				wxString value;
				for (const StructSubItem& subItem: m_SubItems)
				{
					if (value.IsEmpty())
					{
						value = subItem.GetTypeID().ToString();
					}
					else
					{
						value = Kx::Utility::String::ConcatWithSeparator(wxS(", "), value, subItem.GetTypeID().ToString());
					}
				}
				m_CachedViewType = KxString::Format(wxS("struct<%1>"), value);
			}
			return *m_CachedViewType;
		}
		else if (id == ColumnID::Value)
		{
			if (!m_CachedViewValue)
			{
				wxString value;
				for (const StructSubItem& subItem: m_SubItems)
				{
					if (value.IsEmpty())
					{
						value = subItem.GetValue().Serialize(subItem);
					}
					else
					{
						value = Kx::Utility::String::ConcatWithSeparator(wxS(", "), value, subItem.GetValue().Serialize(subItem));
					}
				}
				m_CachedViewValue = KxString::Format(wxS("{%1}"), value);
			}
			return *m_CachedViewValue;
		}
		return Item::GetStringRepresentation(id);
	}

	wxAny StructItem::GetValue(const KxDataView2::Column& column) const
	{
		return Item::GetValue(column);
	}
	wxAny StructItem::GetEditorValue(const KxDataView2::Column& column) const
	{
		if (column.GetID<ColumnID>() == ColumnID::Value)
		{
		}
		return {};
	}
	bool StructItem::SetValue(const wxAny& value, KxDataView2::Column& column)
	{
		if (column.GetID<ColumnID>() == ColumnID::Value)
		{
		}
		return false;
	}

	KxDataView2::Editor* StructItem::GetEditor(const KxDataView2::Column& column) const
	{
		if (column.GetID<ColumnID>() == ColumnID::Value)
		{
			if (!m_Editor)
			{
				m_Editor = CreateEditor();
			}
			return m_Editor.get();
		}
		return nullptr;
	}
	void StructItem::OnActivate(KxDataView2::Column& column)
	{
		if (column.GetID<ColumnID>() == ColumnID::Value)
		{
			Edit(column);
		}
	}
}
