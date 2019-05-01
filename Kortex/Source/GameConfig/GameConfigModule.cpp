#include "stdafx.h"
#include "GameConfigModule.h"
#include <Kortex/GameConfig.hpp>

namespace Kortex
{
	namespace Internal
	{
		const SimpleModuleInfo GameConfigModuleTypeInfo("GameConfigModule", "GameConfigModule.Name", "2.0", ImageResourceID::GearPencil);
	}

	void GameConfigModule::OnInit()
	{
	}
	void GameConfigModule::OnExit()
	{
	}
	void GameConfigModule::OnLoadInstance(IGameInstance& instance, const KxXMLNode& node)
	{
		m_GameConfigManager = CreateGameConfigManager(GetManagerNode<IGameConfigManager>(node));
	}

	std::unique_ptr<Kortex::IGameConfigManager> GameConfigModule::CreateGameConfigManager(const KxXMLNode& node) const
	{
		if (IsEnabledInTemplate(node))
		{
			if (node.GetFirstChildElement("Definitions").GetChildrenCount() > 0)
			{
				return std::make_unique<GameConfig::DefaultGameConfigManager>();
			}
		}
		return nullptr;
	}

	GameConfigModule::GameConfigModule()
		:ModuleWithTypeInfo(Disposition::ActiveInstance)
	{
	}

	IModule::ManagerRefVector GameConfigModule::GetManagers()
	{
		return ToManagersList(m_GameConfigManager);
	}
}
