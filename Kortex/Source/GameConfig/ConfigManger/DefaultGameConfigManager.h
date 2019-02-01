#pragma once
#include "stdafx.h"
#include "GameConfig/IGameConfigManager.h"
#include "Definition.h"
#include <KxFramework/KxTranslation.h>

namespace Kortex::GameInstance
{
	class ProfileEvent;
}

namespace Kortex::GameConfig
{
	class ItemGroup;

	class DefaultGameConfigManager: public IGameConfigManager
	{
		private:
			std::unordered_map<wxString, std::unique_ptr<Definition>> m_Definitions;

			KxTranslation m_Translation;
			RefStackTranslator m_Translator;

		private:
			void LoadGroup(const KxXMLNode& definitionNode, ItemGroup& group);
			void OnChangeProfile(GameInstance::ProfileEvent& event);

		protected:
			void OnInit() override;
			void OnExit() override;
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			
			RefStackTranslator& GetTranslatorStack() override
			{
				return m_Translator;
			}

		public:
			const ITranslator& GetTranslator() const override
			{
				return m_Translator;
			}
	
			void Load() override;
			void Save() override;
	};
}
