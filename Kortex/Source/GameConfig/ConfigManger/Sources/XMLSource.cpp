#include "stdafx.h"
#include "XMLSource.h"
#include "GameConfig/ConfigManger/Item.h"
#include "GameConfig/ConfigManger/ItemValue.h"
#include "GameConfig/ConfigManger/ItemGroup.h"
#include "GameConfig/ConfigManger/Definition.h"
#include "GameConfig/ConfigManger/Items/SimpleItem.h"
#include <KxFramework/KxFileStream.h>

namespace Kortex::GameConfig
{
	bool XMLSource::Open()
	{
		if (!m_IsOpened)
		{
			m_IsOpened = true;
			return true;
		}
		return false;
	}
	bool XMLSource::Save()
	{
		return m_IsOpened;
	}
	void XMLSource::Close()
	{
		m_IsOpened = false;
	}

	bool XMLSource::WriteValue(const Item& item, const ItemValue& value)
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
	bool XMLSource::ReadValue(Item& item, ItemValue& value) const
	{
		if (KxXMLNode node = m_XML.QueryElement(item.GetPath()); node.IsOK())
		{
			wxString attributeName = item.GetName();
			if (attributeName.IsEmpty())
			{
				return value.Deserialize(node.GetValue(), item);
			}
			else
			{
				return value.Deserialize(node.GetAttribute(attributeName), item);
			}
		}
		return false;
	}
	void XMLSource::LoadUnknownItems(ItemGroup& group)
	{
		// Not required currently
	}
}
