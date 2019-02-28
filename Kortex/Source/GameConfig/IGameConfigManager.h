#pragma once
#include "stdafx.h"
#include "IConfigManager.h"
#include <KxFramework/KxSingleton.h>

namespace Kortex
{
	class IGameConfigManager: public IConfigManager, public KxSingletonPtr<IGameConfigManager>
	{
		public:
			IGameConfigManager();
	};
}
