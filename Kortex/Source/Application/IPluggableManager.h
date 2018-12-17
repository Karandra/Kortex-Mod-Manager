#pragma once
#include "stdafx.h"
#include "IManager.h"
class KMainWindow;
class KWorkspace;

namespace Kortex
{
	class IPluggableManager: public IManager
	{
		friend class KMainWindow;

		protected:
			virtual KWorkspace* CreateWorkspace(KMainWindow* mainWindow) = 0;

		public:
			IPluggableManager(IModule* module)
				:IManager(module)
			{
			}

		public:
			const IPluggableManager* ToPluggableManager() const override
			{
				return this;
			}
			IPluggableManager* ToPluggableManager() override
			{
				return this;
			}
	};
}
