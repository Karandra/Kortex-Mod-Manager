#include "stdafx.h"
#include "Item.h"
#include "ItemGroup.h"
#include "Definition.h"

namespace Kortex::GameConfig
{
	Item::Item(ItemGroup& group, const KxXMLNode& itemNode)
		:m_Group(group)
	{
		m_Category = itemNode.GetAttribute(wxS("Category"));
		m_Path = itemNode.GetAttribute(wxS("Path"));
		m_Name = itemNode.GetAttribute(wxS("Name"));

		TypeID type(itemNode.GetAttribute(wxS("Type")));
		m_Value.SetType(group.GetDefinition().GetDataType(type));
	}

	bool Item::IsOK() const
	{
		return !m_Category.IsEmpty() && !m_Path.IsEmpty() && !m_Name.IsEmpty() && m_Value.IsOk();
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
