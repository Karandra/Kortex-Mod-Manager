#include "stdafx.h"
#include "DefaultGameConfigManager.h"
#include "Definition.h"
#include "Workspace.h"
#include "GameInstance/ProfileEvent.h"
#include <Kortex/Application.hpp>

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
	void DefaultGameConfigManager::OnChangeProfile(ProfileEvent& event)
	{
		Load();
		IWorkspace::ScheduleReloadOf<Workspace>();
	}

	void DefaultGameConfigManager::OnInit()
	{
		IConfigManager::OnInit();
		m_BroadcastReciever.Bind(ProfileEvent::EvtSelected, &DefaultGameConfigManager::OnChangeProfile, this);
	}
	void DefaultGameConfigManager::OnExit()
	{
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
				definition->RemoveInvalidGroups();
				m_Definitions.insert_or_assign(id, std::move(definition));
			}
		}
	}
	void DefaultGameConfigManager::CreateWorkspace()
	{
		new Workspace();
	}

	void DefaultGameConfigManager::OnItemChanged(GameConfig::Item& item)
	{
		m_ChangedItems.remove(&item);
		m_ChangedItems.push_back(&item);

		if (Workspace* workspace = Workspace::GetInstance())
		{
			workspace->OnChangesMade();
		}
	}
	void DefaultGameConfigManager::OnItemChangeDiscarded(GameConfig::Item& item)
	{
		m_ChangedItems.remove(&item);

		if (Workspace* workspace = Workspace::GetInstance())
		{
			if (HasUnsavedChanges())
			{
				workspace->OnChangesMade();
			}
			else
			{
				workspace->OnChangesDiscarded();
			}
		}
	}

	IWorkspace::RefVector DefaultGameConfigManager::EnumWorkspaces() const
	{
		return ToWorkspacesList(Workspace::GetInstance());
	}

	void DefaultGameConfigManager::Load()
	{
		m_ChangedItems.clear();

		for (const auto&[id, definition]: m_Definitions)
		{
			definition->ForEachGroup([](ItemGroup& group)
			{
				group.LoadItemsData();
			});
		}
	}
	void DefaultGameConfigManager::SaveChanges()
	{
		std::vector<ISource*> openedSources;
		for (GameConfig::Item* item: m_ChangedItems)
		{
			// Open source for this item if it's not already opened
			ISource& source = item->GetGroup().GetSource();
			if (!source.IsOpened())
			{
				source.Open();
				openedSources.push_back(&source);
			}

			// Save this item
			item->SaveChanges();
		}
		m_ChangedItems.clear();

		// Save and close opened sources
		for (ISource* source: openedSources)
		{
			source->Save();
			source->Close();
		}

		if (Workspace* workspace = Workspace::GetInstance())
		{
			workspace->OnChangesSaved();
		}
	}
	void DefaultGameConfigManager::DiscardChanges()
	{
		for (GameConfig::Item* item: m_ChangedItems)
		{
			item->DiscardChanges();
		}
		m_ChangedItems.clear();

		if (Workspace* workspace = Workspace::GetInstance())
		{
			workspace->OnChangesDiscarded();
		}
	}
	bool DefaultGameConfigManager::HasUnsavedChanges() const
	{
		return !m_ChangedItems.empty();
	}
}
