#include "stdafx.h"
#include "IManager.h"
#include "UI/KWorkspace.h"

namespace Kortex
{
	void IManager::ScheduleReloadWorkspace() const
	{
		if (KWorkspace* workspace = GetWorkspace())
		{
			workspace->ScheduleReload();
		}
	}
}
