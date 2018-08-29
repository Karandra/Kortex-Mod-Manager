#pragma once
#include "stdafx.h"
#include "KManager.h"
class KMainWindow;

class KPluggableManager: public KManager
{
	friend class KMainWindow;

	protected:
		virtual KWorkspace* CreateWorkspace(KMainWindow* mainWindow) = 0;

	public:
		KPluggableManager();
		virtual ~KPluggableManager();

	public:
		virtual const KPluggableManager* ToPluggableManager() const override
		{
			return this;
		}
		virtual KPluggableManager* ToPluggableManager() override
		{
			return this;
		}
};
