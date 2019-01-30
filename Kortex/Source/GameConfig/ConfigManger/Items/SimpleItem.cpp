#include "stdafx.h"
#include "SimpleItem.h"

namespace Kortex::GameConfig
{
	bool SimpleItem::Create(const KxXMLNode& itemNode)
	{
		m_Value.SetType(GetDataType());
		return true;
	}
	void SimpleItem::Load(const ISource& source)
	{
	}
	void SimpleItem::Save(ISource& source) const
	{
	}

	SimpleItem::SimpleItem(ItemGroup& group, const KxXMLNode& itemNode)
		:RTTI::IImplementation<Item>(group, itemNode)
	{
	}
}
