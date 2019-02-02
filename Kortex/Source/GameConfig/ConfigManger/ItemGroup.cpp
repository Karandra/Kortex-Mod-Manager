#include "stdafx.h"
#include "ItemGroup.h"
#include "Item.h"
#include "Items/SimpleItem.h"
#include "Definition.h"
#include "INISource.h"
#include <KxFramework/KxXML.h>

namespace Kortex::GameConfig
{
	void ItemGroup::LoadItems(const KxXMLNode& groupNode)
	{
		for (KxXMLNode node = groupNode.GetFirstChildElement(wxS("Item")); node.IsOK(); node = node.GetNextSiblingElement(wxS("Item")))
		{
			ItemKindValue kind;
			kind.FromString(node.GetAttribute(wxS("Kind")));

			std::unique_ptr<Item> item = nullptr;
			switch (kind.GetValue())
			{
				case ItemKindID::Simple:
				{
					item = NewItem<SimpleItem>(*this, node);
					break;
				}
			};

			if (item && item->Create(groupNode))
			{
				AddItem(std::move(item));
			}
		}
	}

	ItemGroup::ItemGroup(Definition& definition, const wxString& id, const KxXMLNode& groupNode, const ItemOptions& parentOptions)
		:m_Definition(definition), m_ID(id), m_Options(groupNode)
	{
		m_Options.CopyIfNotSpecified(parentOptions);
		LoadItems(groupNode);
	}

	bool ItemGroup::OnLoadInstance(const KxXMLNode& groupNode)
	{
		const KxXMLNode sourceNode = groupNode.GetFirstChildElement(wxS("Source"));

		// Only 'FSPath' source type is currently supported
		m_SourceType = SourceTypeDef::FromString(sourceNode.GetAttribute(wxS("Type")), SourceType::None);
		if (m_SourceType == SourceType::FSPath)
		{
			switch (m_Options.GetSourceFormat().GetValue())
			{
				case SourceFormat::INI:
				{
					m_Source = std::make_unique<INISource>(sourceNode.GetValue());
					return true;
				}
				case SourceFormat::XML:
				{
					//m_Source = std::make_unique<XMLSource>(sourceNode.GetValue());
					break;
				}
				case SourceFormat::Registry:
				{
					//m_Source = std::make_unique<RegistrySource>(sourceNode.GetValue());
					break;
				}
			};
		}
		return false;
	}
	IConfigManager& ItemGroup::GetManager() const
	{
		return m_Definition.GetManager();
	}

	void ItemGroup::ReadItems()
	{
		if (m_Source && m_Source->Open())
		{
			if (!m_UnknownLoaded)
			{
				m_Source->LoadUnknownItems(*this);
				m_UnknownLoaded = true;
			}

			for (auto& [hash, item]: m_Items)
			{
				item->Read(*m_Source);
			}
			m_Source->Close();
		}
	}
	wxAny ItemGroup::GetValue(const Column& column) const
	{
		switch (column.GetID<ColumnID>())
		{
			case ColumnID::Path:
			{
				return m_Source->GetPathDescription();
			}
		}
		return {};
	}
}
