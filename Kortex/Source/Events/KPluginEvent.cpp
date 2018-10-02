#include "stdafx.h"
#include "KPluginEvent.h"
#include "PluginManager/KPluginEntry.h"

wxString KPluginEvent::GetPluginName() const
{
	return m_Plugin ? m_Plugin->GetName() : wxNotifyEvent::GetString();
}

//////////////////////////////////////////////////////////////////////////
wxDEFINE_EVENT(KEVT_PLUGIN_TOGGLED, KPluginEvent);
wxDEFINE_EVENT(KEVT_PLUGIN_CHANGED, KPluginEvent);

wxDEFINE_EVENT(KEVT_PLUGINS_REORDERED, KPluginEvent);
