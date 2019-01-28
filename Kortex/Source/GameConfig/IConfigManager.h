#pragma once
#include "stdafx.h"
#include "Application/IManager.h"
#include "Application/RefStackTranslator.h"
#include <KxFramework/KxTranslation.h>
class KxXMLNode;

namespace Kortex
{
	namespace ConfigManager::Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	};

	class IConfigManager: public ManagerWithTypeInfo<IManager, ConfigManager::Internal::TypeInfo>
	{
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

			wxString LoadItemLabel(const KxXMLNode& itemNode, const wxString& name) const;
	};
}
