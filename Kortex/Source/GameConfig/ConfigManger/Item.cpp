#include "stdafx.h"
#include "Item.h"
#include "ItemGroup.h"

namespace Kortex::GameConfig
{
	Item::Item(ItemGroup& group, const KxXMLNode& itemNode)
		:m_Group(group)
	{
		m_Category = itemNode.GetAttribute(wxS("Category"));
		m_Path = itemNode.GetAttribute(wxS("Path"));
		m_Name = itemNode.GetAttribute(wxS("Name"));
		m_Type.FromString(itemNode.GetAttribute(wxS("Type")));
	}

	bool Item::IsOK() const
	{
		return !m_Category.IsEmpty() && !m_Path.IsEmpty() && !m_Name.IsEmpty() && !m_Type.IsNone() && !m_Type.IsAny();
	}
	
	IConfigManager& Item::GetManager() const
	{
		return m_Group.GetManager();
	}
	Definition& Item::GetDefinition() const
	{
		return m_Group.GetDefinition();
	}
}
