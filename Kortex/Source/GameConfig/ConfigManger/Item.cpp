#include "stdafx.h"
#include "Item.h"
#include "ItemGroup.h"
#include "Definition.h"
#include "GameConfig/IConfigManager.h"

namespace Kortex::GameConfig
{
	Item::Item(ItemGroup& group, const KxXMLNode& itemNode)
		:m_Group(group)
	{
		if (itemNode.IsOK())
		{
			m_Category = itemNode.GetAttribute(wxS("Category"));
			m_Path = itemNode.GetAttribute(wxS("Path"));
			m_Name = itemNode.GetAttribute(wxS("Name"));
			m_Label = GetManager().LoadItemLabel(itemNode, m_Name, wxS("ValueName"));
			m_TypeID.FromString(itemNode.GetAttribute(wxS("Type")));
			m_Kind.FromString(itemNode.GetAttribute(wxS("Kind")));

			m_Options.Load(itemNode.GetFirstChildElement(wxS("Options")), GetDataType());
			m_Options.CopyIfNotSpecified(group.GetOptions());
		}
	}

	bool Item::IsOK() const
	{
		return !m_Category.IsEmpty() && !m_Path.IsEmpty() && m_TypeID.IsDefinitiveType();
	}
	
	IConfigManager& Item::GetManager() const
	{
		return m_Group.GetManager();
	}
	Definition& Item::GetDefinition() const
	{
		return m_Group.GetDefinition();
	}

	void Item::ReadItem()
	{
		Clear();
		Read(m_Group.GetSource());
	}
	DataType Item::GetDataType() const
	{
		return m_Group.GetDefinition().GetDataType(m_TypeID);
	}
}
