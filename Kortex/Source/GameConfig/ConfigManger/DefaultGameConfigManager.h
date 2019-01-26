#pragma once
#include "stdafx.h"
#include "GameConfig/IGameConfigManager.h"

namespace Kortex::GameConfig
{
	class DefaultGameConfigManager: public IGameConfigManager
	{
		private:
			void OnInit() override;
			void OnExit() override;
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
	};
}
