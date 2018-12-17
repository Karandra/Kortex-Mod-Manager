#include "stdafx.h"
#include <Kortex/ModManager.hpp>

namespace Kortex::ModManager
{
	wxString ModEvent::GetModID() const
	{
		return m_Mod ? m_Mod->GetID() : wxNotifyEvent::GetString();
	}
}

namespace Kortex::Events
{
	wxDEFINE_EVENT(ModInstalling, ModManager::ModEvent);
	wxDEFINE_EVENT(ModInstalled, ModManager::ModEvent);

	wxDEFINE_EVENT(ModUnsinstalling, ModManager::ModEvent);
	wxDEFINE_EVENT(ModUninstalled, ModManager::ModEvent);

	wxDEFINE_EVENT(ModToggled, ModManager::ModEvent);
	wxDEFINE_EVENT(ModChanged, ModManager::ModEvent);
	wxDEFINE_EVENT(ModFilesChanged, ModManager::ModEvent);

	wxDEFINE_EVENT(ModsReordered, ModManager::ModEvent);

	wxDEFINE_EVENT(ModVirtualTreeInvalidated, ModManager::ModEvent);
}
