#include "pch.hpp"
#include "IModule.h"
#include "IWorkspace.h"

namespace Kortex
{
	void IModule::ScheduleWorkspacesReload()
	{
		for (IWorkspace& workspace: EnumWorkspaces())
		{
			workspace.ScheduleReload();
		};
	}
}
