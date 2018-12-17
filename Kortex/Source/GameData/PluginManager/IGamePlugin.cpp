#include "stdafx.h"
#include <Kortex/ModManager.hpp>
#include "IGamePlugin.h"
#include "IPluginManager.h"

namespace Kortex
{
	bool IGamePlugin::CanToggleActive() const
	{
		if (const IGameMod* mod = GetOwningMod())
		{
			return mod->IsActive();
		}
		return true;
	}

	intptr_t IGamePlugin::GetOrderIndex() const
	{
		return IPluginManager::GetInstance()->OnGetOrderIndex(*this);
	}
	intptr_t IGamePlugin::GetPriority() const
	{
		return IPluginManager::GetInstance()->OnGetPluginPriority(*this);
	}
	intptr_t IGamePlugin::GetDisplayPriority() const
	{
		return IPluginManager::GetInstance()->OnGetPluginDisplayPriority(*this);
	}

	bool IGamePlugin::HasDependentPlugins() const
	{
		return IPluginManager::GetInstance()->HasDependentPlugins(*this);
	}
	Kortex::IGamePlugin::RefVector IGamePlugin::GetDependentPlugins() const
	{
		return IPluginManager::GetInstance()->GetDependentPlugins(*this);
	}
}
