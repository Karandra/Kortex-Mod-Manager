#pragma once
#include "stdafx.h"
#include "Application/IPluggableManager.h"
#include "Application/RefStackTranslator.h"
#include <KxFramework/KxTranslation.h>
class KxXMLNode;

namespace Kortex
{
	namespace GameConfig
	{
		class Definition;
		class ItemGroup;
		class Item;
	}
	namespace ConfigManager::Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	};

	class IConfigManager: public ManagerWithTypeInfo<IPluggableManager, ConfigManager::Internal::TypeInfo>
	{
		public:
			using DefinitionFunc = std::function<void(const GameConfig::Definition& definition)>;

		private:
			KxTranslation m_Translation;

		protected:
			void OnInit() override;
			void OnExit() override;
			void OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode) override;

			virtual RefStackTranslator& GetTranslatorStack() = 0;
			bool LoadTranslation(KxTranslation& translation, const wxString& component);

		public:
			IConfigManager();

		public:
			virtual const ITranslator& GetTranslator() const = 0;
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
			virtual void Save() = 0;

			wxString TranslateItemLabel(const wxString& name, const wxString& perfix) const;
			wxString TranslateItemLabel(const KxXMLNode& itemNode, const wxString& name, const wxString& perfix) const;
	};
}
