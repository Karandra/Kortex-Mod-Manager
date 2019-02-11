#include "stdafx.h"
#include "WorkspaceController.h"
#include "Workspace.h"
#include <Kortex/GameConfig.hpp>

namespace Kortex::GameConfig
{
	bool WorkspaceController::HasUnsavedChanges() const
	{
		return IGameConfigManager::GetInstance()->HasUnsavedChanges();
	}
	void WorkspaceController::SaveChanges()
	{
		IGameConfigManager::GetInstance()->SaveChanges();
	}
	void WorkspaceController::DiscardChanges()
	{
		IGameConfigManager::GetInstance()->DiscardChanges();
	}
}
