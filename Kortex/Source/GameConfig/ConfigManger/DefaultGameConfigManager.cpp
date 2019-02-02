#include "stdafx.h"
#include "DefaultGameConfigManager.h"
#include "Definition.h"
#include "Workspace.h"
#include <Kortex/Application.hpp>
#include <Kortex/Events.hpp>

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
	void DefaultGameConfigManager::OnChangeProfile(GameInstance::ProfileEvent& event)
	{
		Load();

		if (Workspace* workspace = Workspace::GetInstance())
		{
			workspace->ReloadWorkspace();
		}
	}

	void DefaultGameConfigManager::OnInit()
	{
		IConfigManager::OnInit();
		IEvent::Bind(Events::ProfileSelected, &DefaultGameConfigManager::OnChangeProfile, this);
	}
	void DefaultGameConfigManager::OnExit()
	{
		if (Workspace* workspace = Workspace::GetInstance())
		{
			workspace->ClearView();
		}
		IConfigManager::OnExit();
	}
	void DefaultGameConfigManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		IConfigManager::OnLoadInstance(instance, managerNode);
		if (LoadTranslation(m_Translation, "GameConfig"))
		{
			m_Translator.Push(m_Translation);
		}

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
	KWorkspace* DefaultGameConfigManager::CreateWorkspace(KMainWindow* mainWindow)
	{
		return new Workspace(mainWindow);
	}

	void DefaultGameConfigManager::Load()
	{
		for (const auto& [id, definition]: m_Definitions)
		{
			definition->ForEachGroup([](ItemGroup& group)
			{
				group.ReadItems();
			});
		}
	}
	void DefaultGameConfigManager::Save()
	{
	}
}
