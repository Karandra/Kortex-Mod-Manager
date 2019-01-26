#include "stdafx.h"
#include "GameConfigModule.h"

namespace Kortex
{
	namespace Internal
	{
		const SimpleModuleInfo GameConfigModuleTypeInfo("GameConfigModule", "GameConfigModule.Name", "1.0", KIMG_GEAR_PENCIL);
	}

	void GameConfigModule::OnInit()
	{
	}
	void GameConfigModule::OnExit()
	{
	}
	void GameConfigModule::OnLoadInstance(IGameInstance& instance, const KxXMLNode& node)
	{
	}

	GameConfigModule::GameConfigModule()
		:ModuleWithTypeInfo(Disposition::ActiveInstance)
	{
	}

	IModule::ManagerRefVector GameConfigModule::GetManagers()
	{
		return {};
	}
}
