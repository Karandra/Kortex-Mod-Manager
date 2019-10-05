#pragma once
#include "stdafx.h"
#include "Application/IManager.h"
#include "Application/RefStackTranslator.h"
#include "ConfigManger/ItemValue.h"
#include "ConfigManger/IAction.h"
#include "ConfigManger/ISamplingFunction.h"
#include <KxFramework/KxTranslation.h>
class KxXMLNode;

namespace Kortex
{
	namespace GameConfig
	{
		class DisplayModel;
		class Definition;
		class ItemGroup;
		class Item;
	}
	namespace ConfigManager::Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	};

	class IConfigManager: public ManagerWithTypeInfo<IManager, ConfigManager::Internal::TypeInfo>
	{
		friend class GameConfig::Item;
		friend class GameConfig::DisplayModel;

		public:
			using DefinitionFunc = std::function<void(const GameConfig::Definition& definition)>;

		public:
			static wxString GetDefinitionFileByID(const wxString& id);

		private:
			KxTranslation m_Translation;

		protected:
			void OnInit() override;
			void OnExit() override;
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;
			
			virtual void OnCreateDisplayModel(GameConfig::DisplayModel& displayModel)
			{
			}
			virtual void OnDestroyDisplayModel(GameConfig::DisplayModel& displayModel)
			{
			}

			virtual RefStackTranslator& GetTranslatorStack() = 0;
			bool LoadTranslation(KxTranslation& translation, const wxString& component);
			bool LoadDefaultTranslation();

			virtual void OnItemChanged(GameConfig::Item& item) = 0;
			virtual void OnItemChangeDiscarded(GameConfig::Item& item) = 0;

		public:
			IConfigManager();

		public:
			virtual const ITranslator& GetTranslator() const = 0;
			virtual GameConfig::DisplayModel* GetDisplayModel() const = 0;

			virtual void ForEachDefinition(const DefinitionFunc& func) = 0;
			template<class TFunctor> void ForEachGroup(TFunctor&& func)
			{
				ForEachDefinition([&func](auto&& definition)
				{
					definition.ForEachGroup(func);
				});
			}
			template<class TFunctor> void ForEachItem(TFunctor&& func)
			{
				ForEachGroup([&func](auto&& group)
				{
					group.ForEachItem(func);
				});
			}

			virtual void Load() = 0;
			virtual void SaveChanges() = 0;
			virtual void DiscardChanges() = 0;
			virtual bool HasUnsavedChanges() const = 0;

			wxString TranslateItemLabel(const wxString& name, const wxString& perfix) const;
			wxString TranslateItemLabel(const KxXMLNode& itemNode, const wxString& name, const wxString& perfix, bool isAttribute = false) const;
			std::pair<wxString, bool> TranslateItemElement(const KxXMLNode& itemNode, bool isAttribute = false, const wxString& attributeName = {}) const;

			virtual std::unique_ptr<GameConfig::IAction> QueryAction(const wxString& name)
			{
				return nullptr;
			}
			virtual std::unique_ptr<GameConfig::ISamplingFunction> QuerySamplingFunction(const wxString& name, GameConfig::SampleValue::Vector& samples)
			{
				return nullptr;
			}
	};
}
