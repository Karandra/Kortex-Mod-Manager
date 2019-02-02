#include "stdafx.h"
#include "Item.h"
#include "ItemGroup.h"
#include "Definition.h"
#include "GameConfig/IConfigManager.h"
#include <KxStringUtility.h>

namespace Kortex::GameConfig
{
	size_t HashStore::Get(const Item& item, const wxString& value) const
	{
		if (!m_Hash)
		{
			m_Hash = item.CalcHash(value);
		}
		return *m_Hash;
	}
}

namespace Kortex::GameConfig
{
	size_t Item::CalcHash(const wxString& value) const
	{
		wxString hashData = m_Path;
		hashData += wxS('/');
		hashData += m_Name;

		if (!value.IsEmpty())
		{
			hashData += wxS('/');
			hashData += value;
		}
		return std::hash<wxString>()(hashData);
	}

	Item::Item(ItemGroup& group, const KxXMLNode& itemNode)
		:m_Group(group), m_Samples(*this)
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

			m_Samples.Load(itemNode.GetFirstChildElement(wxS("Samples")));
		}
	}

	bool Item::IsOK() const
	{
		return !m_Category.IsEmpty() && !m_Path.IsEmpty() && m_TypeID.IsDefinitiveType();
	}
	wxString Item::GetFullPath() const
	{
		return Kx::Utility::String::ConcatWithSeparator(wxS('/'), m_Group.GetSource().GetPathDescription(), m_Path, m_Name);
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
