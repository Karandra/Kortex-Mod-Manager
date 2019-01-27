#include "stdafx.h"
#include "ItemSamples.h"
#include <KxFramework/KxXML.h>

namespace Kortex::GameConfig
{
	void ItemSamples::LoadSamples(const KxXMLNode& rootNode)
	{
		for (KxXMLNode node = rootNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{

		}
	}

	ItemSamples::ItemSamples(Item& item, const KxXMLNode& node)
		:m_Item(item)
	{
		m_SortOrder.FromString(node.GetAttribute(wxS("SortOrder")));
		m_SortOptions.FromOrExpression(node.GetAttribute(wxS("SortOptions")));

		m_Values.reserve(node.GetChildrenCount());
		LoadSamples(node);
	}
}
