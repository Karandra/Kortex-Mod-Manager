#include "stdafx.h"
#include "ConfigManager.h"
#include "GameConfig/ConfigManger/Sources/XMLRefSource.h"
#include <Kortex/Application.hpp>
#include <Kortex/GameInstance.hpp>
#include "Application/SystemApplication.h"
#include <KxFramework/KxFileFinder.h>

namespace Kortex::Application::Settings
{
	void ConfigManager::OnInit()
	{
		IConfigManager::OnInit();
		IConfigManager::LoadDefaultTranslation();
		
		m_Definition.Load();

		if (GameConfig::ItemGroup* group = m_Definition.GetGroupByID("Global"))
		{
			group->AssignSource(std::make_unique<GameConfig::XMLRefSource>(SystemApplication::GetInstance()->GetGlobalConfig()));
		}
		if (GameConfig::ItemGroup* group = m_Definition.GetGroupByID("Instance"))
		{
			IConfigurableGameInstance* configurableInstance = nullptr;
			if (IGameInstance* activeInstance = IGameInstance::GetActive(); activeInstance && activeInstance->QueryInterface(configurableInstance))
			{
				group->AssignSource(std::make_unique<GameConfig::XMLRefSource>(configurableInstance->GetConfig()));
			}
		}
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
		m_ChangedItems.clear();
		m_Definition.ForEachGroup([](GameConfig::ItemGroup& group)
		{
			group.LoadItemsData();
		});
	}
	void ConfigManager::SaveChanges()
	{
		using namespace GameConfig;

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
	}
	void ConfigManager::DiscardChanges()
	{
		for (GameConfig::Item* item: m_ChangedItems)
		{
			item->DiscardChanges();
		}
		m_ChangedItems.clear();
	}
	bool ConfigManager::HasUnsavedChanges() const
	{
		return !m_ChangedItems.empty();
	}

	std::unique_ptr<GameConfig::ISamplingFunction> ConfigManager::QuerySamplingFunction(const wxString& name, GameConfig::SampleValue::Vector& samples)
	{
		using namespace GameConfig;

		if (name == wxS("VirtualFileSystem/GetLibraries"))
		{
			return MakeGenericSamplingFunction([&samples](const ItemValue::Vector& arguments)
			{
				KxFileFinder finder(IApplication::GetInstance()->GetDataFolder() + wxS("\\VFS\\Library\\*"));
				for (KxFileItem item = finder.FindNext(); item.IsOK(); item = finder.FindNext())
				{
					if (item.IsDirectory() && item.IsNormalItem())
					{
						samples.emplace_back(item.GetName());
					}
				}
			});
		}
		return nullptr;
	}
}
