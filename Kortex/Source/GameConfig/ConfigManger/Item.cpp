#include "stdafx.h"
#include "Item.h"
#include "ItemGroup.h"
#include "Definition.h"
#include "GameConfig/IConfigManager.h"
#include <KxFramework/KxStringUtility.h>
#include <KxFramework/KxComparator.h>

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
			m_Label = GetManager().TranslateItemLabel(itemNode, m_Name, wxS("ValueName"));
			m_TypeID.FromString(itemNode.GetAttribute(wxS("Type")));
			m_Kind.FromString(itemNode.GetAttribute(wxS("Kind")));

			m_Options.Load(itemNode.GetFirstChildElement(wxS("Options")), GetDataType());
			m_Options.CopyIfNotSpecified(group.GetOptions(), GetDataType());

			m_Samples.Load(itemNode.GetFirstChildElement(wxS("Samples")));
		}
	}
	Item::~Item()
	{
		DetachAllChildren();
	}

	bool Item::IsOK() const
	{
		return !m_Category.IsEmpty() && !m_Path.IsEmpty() && m_TypeID.IsDefinitiveType();
	}
	wxString Item::GetFullPath() const
	{
		return Kx::Utility::String::ConcatWithSeparator(wxS('/'), m_Group.GetSource().GetPathDescription(), m_Path, m_Name);
	}
	wxString Item::GetStringRepresentation(ColumnID id) const
	{
		switch (id)
		{
			case ColumnID::Path:
			{
				return m_Path;
			}
			case ColumnID::Name:
			{
				return m_Name;
			}
			case ColumnID::Type:
			{
				return m_TypeID.ToString();
			}
		};
		return {};
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
	bool Item::IsEditable() const
	{
		if (m_Options.IsAutoEditable())
		{
			return IsUnknown() || m_TypeID.IsFloat() || (m_TypeID.IsString() || m_TypeID.IsInteger() && m_Samples.IsEmpty());
		}
		return m_Options.IsEditable();
	}

	bool Item::Compare(const KxDataView2::Node& node, const KxDataView2::Column& column) const
	{
		const IViewItem* viewItem = nullptr;
		if (node.QueryInterface(viewItem))
		{
			const ColumnID id = column.GetID<ColumnID>();
			return KxComparator::IsLess(GetStringRepresentation(id), viewItem->GetStringRepresentation(id), true);
		}
		return false;
	}
}
