#pragma once
#include "stdafx.h"
#include "GameConfig/IGameConfigManager.h"
#include "Definition.h"
#include <KxFramework/KxTranslation.h>

namespace Kortex
{
	class ProfileEvent;
}

namespace Kortex::GameConfig
{
	class ItemGroup;

	class DefaultGameConfigManager: public IGameConfigManager
	{
		private:
			BroadcastReciever m_BroadcastReciever;
			std::unordered_map<wxString, std::unique_ptr<Definition>> m_Definitions;

			KxTranslation m_Translation;
			RefStackTranslator m_Translator;
			std::list<GameConfig::Item*> m_ChangedItems;
			DisplayModel* m_DisplayModel = nullptr;

		private:
			void LoadGroup(const KxXMLNode& definitionNode, ItemGroup& group);
			void OnChangeProfile(ProfileEvent& event);

		protected:
			void OnInit() override;
			void OnExit() override;
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			KWorkspace* CreateWorkspace(KMainWindow* mainWindow) override;

			void OnCreateDisplayModel(DisplayModel& displayModel) override
			{
				m_DisplayModel = &displayModel;
			}
			void OnDestroyDisplayModel(DisplayModel& displayModel) override
			{
				m_DisplayModel = nullptr;
			}

			RefStackTranslator& GetTranslatorStack() override
			{
				return m_Translator;
			}
			void OnItemChanged(GameConfig::Item& item) override;
			void OnItemChangeDiscarded(GameConfig::Item& item) override;

		public:
			const ITranslator& GetTranslator() const override
			{
				return m_Translator;
			}
			DisplayModel* GetDisplayModel() const override
			{
				return m_DisplayModel;
			}
			void ForEachDefinition(const DefinitionFunc& func) override
			{
				for (const auto& [id, definition]: m_Definitions)
				{
					func(*definition);
				}
			}

			void Load() override;
			void SaveChanges() override;
			void DiscardChanges() override;
			bool HasUnsavedChanges() const override;
	};
}
