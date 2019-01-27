#pragma once
#include "stdafx.h"
#include "GameConfig/IGameConfigManager.h"
#include "Definition.h"

namespace Kortex::GameConfig
{
	class ItemGroup;

	class DefaultGameConfigManager: public IGameConfigManager
	{
		private:
			std::unordered_map<wxString, std::unique_ptr<Definition>> m_Definitions;

		private:
			void LoadGroup(const KxXMLNode& definitionNode, ItemGroup& group);

		protected:
			void OnInit() override;
			void OnExit() override;
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;

		public:
			virtual ~DefaultGameConfigManager();
	};
}
