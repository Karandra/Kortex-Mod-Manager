#include "stdafx.h"
#include "PluginEvent.h"
#include <Kortex/PluginManager.hpp>

namespace Kortex::PluginManager
{
	wxString PluginEvent::GetPluginName() const
	{
		return m_Plugin ? m_Plugin->GetName() : wxNotifyEvent::GetString();
	}
}

namespace Kortex::Events
{
	wxDEFINE_EVENT(PluginToggled, PluginManager::PluginEvent);
	wxDEFINE_EVENT(PluginChanged, PluginManager::PluginEvent);

	wxDEFINE_EVENT(PluginsReordered, PluginManager::PluginEvent);
}
