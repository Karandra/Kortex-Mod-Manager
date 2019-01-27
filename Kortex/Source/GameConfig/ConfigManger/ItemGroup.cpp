#include "stdafx.h"
#include "ItemGroup.h"
#include "Item.h"
#include "Definition.h"
#include "INISource.h"
#include <KxFramework/KxXML.h>

namespace Kortex::GameConfig
{
	void ItemGroup::LoadItems(const KxXMLNode& groupNode)
	{
		for (KxXMLNode node = groupNode.GetFirstChildElement(wxS("Item")); node.IsOK(); node = node.GetNextSiblingElement(wxS("Item")))
		{
			m_Items.emplace_back(std::make_unique<Item>(*this, node));
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
}
