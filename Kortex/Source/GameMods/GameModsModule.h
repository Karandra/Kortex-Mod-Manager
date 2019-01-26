#pragma once
#include "stdafx.h"
#include <KxFramework/KxSingleton.h>

namespace Kortex
{
	namespace Internal
	{
		extern const SimpleModuleInfo GameModsTypeInfo;
	}

	class IModManager;
	class IModDispatcher;
	class IModTagManager;
	class IModStatistics;

	class GameModsModule:
		public ModuleWithTypeInfo<IModule, Internal::GameModsTypeInfo>,
		public KxSingletonPtr<GameModsModule>
	{
		private:
			std::unique_ptr<IModManager> m_ModManager;
			std::unique_ptr<IModDispatcher> m_ModDispatcher;
			std::unique_ptr<IModTagManager> m_TagManager;
			std::unique_ptr<IModStatistics> m_ModStatistics;

		protected:
			virtual void OnLoadInstance(IGameInstance& instance, const KxXMLNode& node) override;
			virtual void OnInit() override;
			virtual void OnExit() override;

		public:
			GameModsModule();

		public:
			virtual ManagerRefVector GetManagers() override;
	};
}
