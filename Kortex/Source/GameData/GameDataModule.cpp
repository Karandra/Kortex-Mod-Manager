#include "stdafx.h"
#include <Kortex/PluginManager.hpp>
#include <Kortex/SaveManager.hpp>
#include <Kortex/ScreenshotsGallery.hpp>
#include "Utility/Log.h"

namespace Kortex
{
	namespace Internal
	{
		const SimpleModuleInfo GameDataTypeInfo("GameData", "GameDataModule.Name", "1.3", ImageResourceID::PlugDisconnect);
	}

	void GameDataModule::OnLoadInstance(IGameInstance& instance, const KxXMLNode& node)
	{
		m_PluginManager = CreatePluginManager(GetManagerNode<IPluginManager>(node));
		m_SaveManager = CreateManagerIfEnabled<SaveManager::DefaultSaveManager>(node);
		m_ScreenshotsGallery = CreateManagerIfEnabled<ScreenshotsGallery::DefaultScreenshotsGallery>(node);
	}
	void GameDataModule::OnInit()
	{
	}
	void GameDataModule::OnExit()
	{
	}

	std::unique_ptr<IPluginManager> GameDataModule::CreatePluginManager(const KxXMLNode& node) const
	{
		using namespace PluginManager;
		using namespace PluginManager::Internal;

		if (IsEnabledInTemplate(node))
		{
			const wxString name = node.GetAttribute("Implementation");
			if (name == ManagerImplementation::Bethesda)
			{
				return std::make_unique<BethesdaPluginManager>();
			}
			else if (name == ManagerImplementation::Bethesda2)
			{
				return std::make_unique<BethesdaPluginManager2>();
			}
			else if (name == ManagerImplementation::BethesdaMorrowind)
			{
				return std::make_unique<BethesdaPluginManagerMW>();
			}
			else
			{
				Utility::Log::LogMessage("GameDataModule::CreatePluginManager: Unknown interface requested \"%1\"", name);
			}
		}
		return nullptr;
	}

	GameDataModule::GameDataModule()
		:ModuleWithTypeInfo(Disposition::ActiveInstance)
	{
	}

	IModule::ManagerRefVector GameDataModule::GetManagers()
	{
		return ToManagersList(m_PluginManager, m_SaveManager, m_ScreenshotsGallery);
	}
}
