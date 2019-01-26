#pragma once
#include "stdafx.h"
#include "Application/IManager.h"

namespace Kortex
{
	namespace ConfigManager::Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	};

	class IConfigManager: public ManagerWithTypeInfo<IManager, ConfigManager::Internal::TypeInfo>
	{
		public:
			IConfigManager();
	};
}
