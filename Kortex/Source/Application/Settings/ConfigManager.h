#pragma once
#include "stdafx.h"
#include "GameConfig/IConfigManager.h"
#include "GameConfig/ConfigManger/Definition.h"
#include "GameConfig/ConfigManger/ItemGroup.h"
#include "GameConfig/ConfigManger/Item.h"
#include "GameConfig/ConfigManger/ISource.h"
#include <KxFramework/KxTranslation.h>
class KWorkspace;
class KMainWindow;

namespace Kortex::Application::Settings
{
	class ConfigManager: public IConfigManager
	{
		friend class Window;

		private:
			GameConfig::Definition m_Definition;
			KxTranslation m_Translation;
			RefStackTranslator m_Translator;
			bool m_HasChanges = false;

		protected:
			void OnInit() override;
			void OnExit() override;
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			KWorkspace* CreateWorkspace(KMainWindow* mainWindow) override
			{
				return nullptr;
			}

			RefStackTranslator& GetTranslatorStack() override
			{
				return m_Translator;
			}
			void OnItemChanged(GameConfig::Item& item) override
			{
				m_HasChanges = true;
			}
			void OnItemChangeDiscarded(GameConfig::Item& item) override
			{
			}

		public:
			ConfigManager();

		public:
			const ITranslator& GetTranslator() const override
			{
				return m_Translator;
			}
			void ForEachDefinition(const DefinitionFunc& func) override
			{
				func(m_Definition);
			}

			void Load() override;
			void SaveChanges() override;
			void DiscardChanges() override;
			bool HasUnsavedChanges() const override;
	};
}
