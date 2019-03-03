#include "stdafx.h"
#include "XMLRefSource.h"
#include "GameConfig/ConfigManger/Item.h"
#include "GameConfig/ConfigManger/ItemValue.h"
#include "GameConfig/ConfigManger/ItemGroup.h"
#include "GameConfig/ConfigManger/Definition.h"
#include "GameConfig/ConfigManger/Items/SimpleItem.h"
#include <KxFramework/KxFileStream.h>

namespace Kortex::GameConfig
{
	bool XMLRefSource::Open()
	{
		if (!m_IsOpened)
		{
			m_IsOpened = true;
			return true;
		}
		return false;
	}
	bool XMLRefSource::Save()
	{
		return m_IsOpened;
	}
	void XMLRefSource::Close()
	{
		m_IsOpened = false;
	}

	bool XMLRefSource::WriteValue(const Item& item, const ItemValue& value)
	{
		if (value.IsNull())
		{
			if (KxXMLNode node = m_XML.QueryElement(item.GetPath()); node.IsOK())
			{
				wxString attributeName = item.GetName();
				if (attributeName.IsEmpty())
				{
					return m_XML.RemoveNode(node);
				}
				else
				{
					return node.RemoveAttribute(attributeName);
				}
			}
		}
		else
		{
			if (KxXMLNode node = m_XML.QueryOrCreateElement(item.GetPath()); node.IsOK())
			{
				wxString attributeName = item.GetName();
				if (attributeName.IsEmpty())
				{
					return node.SetValue(value.Serialize(item));
				}
				else
				{
					return node.SetAttribute(attributeName, value.Serialize(item));
				}
			}
		}
		return false;
	}
	bool XMLRefSource::ReadValue(Item& item, ItemValue& value) const
	{
		if (KxXMLNode node = m_XML.QueryElement(item.GetPath()); node.IsOK())
		{
			wxString attributeName = item.GetName();
			if (!attributeName.IsEmpty())
			{
				// Check if we need to find array node with specified attribute value
				if (attributeName.find(wxS('=')) != wxString::npos)
				{
					wxString attributeValue;
					attributeName = attributeName.BeforeFirst(wxS('='), &attributeValue);

					wxString arrayItemName = item.GetPath().AfterLast(wxS('/'));
					KxXMLNode arrayNode = node.GetParent();
					for (KxXMLNode itemNode = arrayNode.GetFirstChildElement(arrayItemName); itemNode.IsOK(); itemNode = itemNode.GetNextSiblingElement(arrayItemName))
					{
						if (itemNode.GetAttribute(attributeName) == attributeValue)
						{
							return value.Deserialize(itemNode.GetValue(), item);
						}
					}
				}
				else
				{
					return value.Deserialize(node.GetAttribute(attributeName), item);
				}
			}
			else
			{
				return value.Deserialize(node.GetValue(), item);
			}
		}
		return false;
	}
	void XMLRefSource::LoadUnknownItems(ItemGroup& group)
	{
		// Not required currently
	}
}
