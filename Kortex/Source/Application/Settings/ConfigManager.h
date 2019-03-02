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
			
			std::list<GameConfig::Item*> m_ChangedItems;
			GameConfig::DisplayModel* m_DisplayModel = nullptr;

		protected:
			void OnInit() override;
			void OnExit() override;
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			KWorkspace* CreateWorkspace(KMainWindow* mainWindow) override
			{
				return nullptr;
			}

			void OnCreateDisplayModel(GameConfig::DisplayModel& displayModel) override
			{
				m_DisplayModel = &displayModel;
			}
			void OnDestroyDisplayModel(GameConfig::DisplayModel& displayModel) override
			{
				m_DisplayModel = nullptr;
			}

			RefStackTranslator& GetTranslatorStack() override
			{
				return m_Translator;
			}
			void OnItemChanged(GameConfig::Item& item) override
			{
				m_ChangedItems.remove(&item);
				m_ChangedItems.push_back(&item);
			}
			void OnItemChangeDiscarded(GameConfig::Item& item) override
			{
				m_ChangedItems.remove(&item);
			}

		public:
			ConfigManager();

		public:
			const ITranslator& GetTranslator() const override
			{
				return m_Translator;
			}
			GameConfig::DisplayModel* GetDisplayModel() const override
			{
				return m_DisplayModel;
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
