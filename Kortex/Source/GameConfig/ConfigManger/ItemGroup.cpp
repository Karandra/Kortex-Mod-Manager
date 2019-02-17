#include "stdafx.h"
#include "ItemGroup.h"
#include "Definition.h"
#include "Item.h"
#include "Items/SimpleItem.h"
#include "Items/StructItem.h"
#include "Sources/INISource.h"
#include "Sources/NullSource.h"
#include <KxFramework/KxXML.h>

namespace Kortex::GameConfig
{
	void ItemGroup::LoadItems(const KxXMLNode& groupNode)
	{
		for (KxXMLNode node = groupNode.GetFirstChildElement(wxS("Item")); node.IsOK(); node = node.GetNextSiblingElement(wxS("Item")))
		{
			std::unique_ptr<Item> item;

			TypeID type;
			if (type.FromString(node.GetAttribute(wxS("Type"))))
			{
				if (type.IsStruct())
				{
					item = NewItem<StructItem>(*this, node);
				}
				else if (type.IsScalarType())
				{
					item = NewItem<SimpleItem>(*this, node);
				}
			}

			if (item && item->Create(node))
			{
				AddItem(std::move(item));
			}
		}
	}
	void ItemGroup::AddKnownItem(size_t hash, Item& item)
	{
		if (!m_Destructing && hash != 0)
		{
			m_ItemsHash.emplace(hash, &item);
		}
	}
	void ItemGroup::RemoveKnownItem(size_t hash)
	{
		if (!m_Destructing && hash != 0)
		{
			m_ItemsHash.erase(hash);
		}
	}

	ItemGroup::ItemGroup(Definition& definition, const wxString& id, const KxXMLNode& groupNode, const ItemOptions& parentOptions)
		:m_Definition(definition), m_ID(id), m_Options(groupNode)
	{
		m_Options.CopyIfNotSpecified(parentOptions);
		LoadItems(groupNode);
	}
	ItemGroup::~ItemGroup()
	{
		m_Destructing = true;
	}

	IConfigManager& ItemGroup::GetManager() const
	{
		return m_Definition.GetManager();
	}
	void ItemGroup::OnLoadInstance(const KxXMLNode& groupNode)
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
					break;
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
		
		if (!m_Source)
		{
			m_Source = std::make_unique<NullSource>();
		}
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

			for (auto& item: m_Items)
			{
				item->Read(*m_Source);

				// Read struct sub-items if this is a struct
				StructItem* structItem = nullptr;
				if (item->QueryInterface(structItem))
				{
					structItem->ForEachSubItem([this](Item& subItem)
					{
						subItem.Read(*m_Source);
					});
				}
			}
			m_Source->Close();
		}
	}
}
