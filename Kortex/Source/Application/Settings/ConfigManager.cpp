#include "stdafx.h"
#include "ConfigManager.h"
#include "GameConfig/ConfigManger/Sources/NullSource.h"
#include <Kortex/Application.hpp>

namespace Kortex::Application::Settings
{
	void ConfigManager::OnInit()
	{
		IConfigManager::OnInit();
		
		m_Definition.Load();
		m_Definition.ForEachGroup([](GameConfig::ItemGroup& group)
		{
			group.AssignSource(std::make_unique<GameConfig::NullSource>());
		});
	}
	void ConfigManager::OnExit()
	{
		IConfigManager::OnExit();
	}
	void ConfigManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		IConfigManager::OnLoadInstance(instance, managerNode);
	}
	
	ConfigManager::ConfigManager()
		:m_Definition(*this, wxS("AppSettings"), GetDefinitionFileByID(wxS("AppSettings")))
	{
	}

	void ConfigManager::Load()
	{
		m_Definition.ForEachGroup([](GameConfig::ItemGroup& group)
		{
			group.LoadItemsData();
		});
	}
	void ConfigManager::SaveChanges()
	{
		using namespace GameConfig;

		std::vector<ISource*> openedSources;
		ForEachItem([&openedSources](Item& item)
		{
			// Open source for this item if it's not already opened
			ISource& source = item.GetGroup().GetSource();
			if (!source.IsOpened())
			{
				source.Open();
				openedSources.push_back(&source);
			}

			// Save this item
			item.SaveChanges();
		});

		// Save and close opened sources
		for (ISource* source: openedSources)
		{
			source->Save();
			source->Close();
		}
	}
	void ConfigManager::DiscardChanges()
	{
	}
	bool ConfigManager::HasUnsavedChanges() const
	{
		return m_HasChanges;
	}
}
