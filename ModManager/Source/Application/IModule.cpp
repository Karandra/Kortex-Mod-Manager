#include "pch.hpp"
#include "IModule.h"
#include "IWorkspace.h"

namespace Kortex
{
	void IModule::ScheduleWorkspacesReload()
	{
		EnumWorkspaces([](IWorkspace& workspace)
		{
			workspace.ScheduleReload();
			return true;
		});
	}
}
