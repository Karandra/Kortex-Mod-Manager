#pragma once
#include "stdafx.h"
#include "Application/IPluggableManager.h"
#include <KxFramework/KxSingleton.h>

namespace Kortex
{
	namespace SaveManager
	{
		class Config;
	}
	namespace SaveManager::Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	}

	class IGameSave;
	class ISaveManager:
		public ManagerWithTypeInfo<IPluggableManager, SimpleManagerInfo, SaveManager::Internal::TypeInfo>,
		public KxSingletonPtr<ISaveManager>
	{
		public:
			ISaveManager();

		public:
			virtual const SaveManager::Config& GetConfig() const = 0;
			virtual std::unique_ptr<IGameSave> NewSave() const = 0;
	};
}
