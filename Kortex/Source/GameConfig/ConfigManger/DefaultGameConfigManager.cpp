#include "stdafx.h"
#include "DefaultGameConfigManager.h"
#include "Definition.h"

namespace Kortex::GameConfig
{
	void DefaultGameConfigManager::LoadGroup(const KxXMLNode& definitionNode, ItemGroup& group)
	{
		KxXMLNode groupsNode = definitionNode.GetFirstChildElement(wxS("Groups"));
		for (KxXMLNode node = groupsNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{
			if (node.GetAttribute(wxS("ID")) == group.GetID())
			{
				group.OnLoadInstance(node);
				return;
			}
		}
	}

	void DefaultGameConfigManager::OnInit()
	{
	}
	void DefaultGameConfigManager::OnExit()
	{
	}
	void DefaultGameConfigManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		const KxXMLNode definitionsNode = managerNode.GetFirstChildElement("Definitions");
		for (KxXMLNode defNode = definitionsNode.GetFirstChildElement(); defNode.IsOK(); defNode = defNode.GetNextSiblingElement())
		{
			wxString id = defNode.GetAttribute("ID");

			auto definition = std::make_unique<Definition>(*this, id, GetDefinitionFileByID(id));
			if (definition->Load())
			{
				definition->ForEachGroup([this, &defNode](ItemGroup& group)
				{
					LoadGroup(defNode, group);
				});
				m_Definitions.insert_or_assign(id, std::move(definition));
			}
		}
	}

	DefaultGameConfigManager::~DefaultGameConfigManager()
	{
	}
}
