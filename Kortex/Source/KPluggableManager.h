#pragma once
#include "stdafx.h"
#include "KManager.h"
class KMainWindow;

class KPluggableManager: public KManager
{
	friend class KMainWindow;

	protected:
		virtual KWorkspace* CreateWorkspace(KMainWindow* mainWindow) = 0;
		virtual KPluggableManager* ToPluggableManager() override
		{
			return this;
		}

	public:
		KPluggableManager();
		virtual ~KPluggableManager();
};
