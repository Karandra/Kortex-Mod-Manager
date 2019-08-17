#include "stdafx.h"
#include "OptionStore.h"

namespace Kortex::NetworkManager::NXMHandler
{
	void OptionStore::Save(AppOption& option) const
	{
		KxXMLNode rootNode = option.GetNode();
		rootNode.ClearChildren();

		for (auto& [nexusID, anyValue]: m_Options)
		{
			KxXMLNode itemNode = rootNode.NewElement("Item");
			itemNode.SetAttribute("NexusID", nexusID);

			if (const Instance* value = std::get_if<Instance>(&anyValue))
			{
				itemNode.NewElement("Instance").SetAttribute("ID", value->ID);
			}
			else if (const Command* value = std::get_if<Command>(&anyValue))
			{
				KxXMLNode node = itemNode.NewElement("Command");
				node.NewElement("Executable").SetValue(value->Executable);
				node.NewElement("Arguments").SetValue(value->Arguments);
			}
		}
	}
	void OptionStore::Load(const AppOption& option)
	{
		KxXMLNode rootNode = option.GetNode();
		for (KxXMLNode itemNode = rootNode.GetFirstChildElement(); itemNode.IsOK(); itemNode = itemNode.GetNextSiblingElement())
		{
			wxString nexusID = itemNode.GetAttribute("NexusID");
			if (!nexusID.IsEmpty())
			{
				KxString::MakeLower(nexusID);
				if (KxXMLNode node = itemNode.GetFirstChildElement("Instance"); node.IsOK())
				{
					Instance instance{node.GetAttribute("ID")};
					if (instance)
					{
						m_Options.insert_or_assign(nexusID, std::move(instance));
					}
				}
				else if (KxXMLNode node = itemNode.GetFirstChildElement("Command"); node.IsOK())
				{
					Command command;
					command.Executable = node.GetFirstChildElement("Executable").GetValue();
					command.Arguments = node.GetFirstChildElement("Arguments").GetValue();

					if (command)
					{
						m_Options.insert_or_assign(nexusID, std::move(command));
					}
				}
			}
		}
	}
}
