#include "pch.hpp"
#include "IManager.h"
#include "IWorkspace.h"

namespace Kortex
{
	void IManager::ScheduleWorkspacesReload()
	{
		EnumWorkspaces([](IWorkspace& workspace)
		{
			workspace.ScheduleReload();
			return true;
		});
	}
}
