#include "stdafx.h"
#include "SimpleItem.h"

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
		:RTTI::IImplementation<Item>(group, itemNode)
	{
	}
}
