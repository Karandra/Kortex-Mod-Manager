#include "stdafx.h"
#include "ItemGroup.h"
#include "INISource.h"
#include <KxFramework/KxXML.h>

namespace Kortex::GameConfig
{
	ItemGroup::ItemGroup(const wxString& id, const KxXMLNode& groupNode, const ItemOptions& parentOptions)
		:m_ID(id), m_Options(groupNode)
	{
		m_Options.CopyIfNotSpecified(parentOptions);
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
					break;
				}
				case SourceFormat::Registry:
				{
					break;
				}
			};
		}
		return false;
	}
}
