#pragma once
#include "stdafx.h"
#include "KEvent.h"
class KPluginEntry;

class KPluginEvent: public KEvent
{
	public:
		using RefVector = std::vector<KPluginEntry*>;

	private:
		KPluginEntry* m_Plugin = NULL;
		RefVector m_PluginsArray;

	public:
		KPluginEvent(wxEventType type = wxEVT_NULL)
			:KEvent(type)
		{
		}
		KPluginEvent(wxEventType type, KPluginEntry& plugin)
			:KEvent(type), m_Plugin(&plugin)
		{
		}
		KPluginEvent(wxEventType type, RefVector pluginsArray)
			:KEvent(type), m_PluginsArray(pluginsArray)
		{
		}
		KPluginEvent(wxEventType type, const wxString& name)
			:KEvent(type)
		{
			wxNotifyEvent::SetString(name);
		}

		KPluginEvent* Clone() const override
		{
			return new KPluginEvent(*this);
		}

	public:
		bool HasPlugin() const
		{
			return m_Plugin != NULL;
		}
		KPluginEntry* GetMod() const
		{
			return m_Plugin;
		}
		wxString GetPluginName() const;

		bool HasPluginsArray() const
		{
			return !m_PluginsArray.empty();
		}
		const RefVector& GetPluginsArray() const
		{
			return m_PluginsArray;
		}

};

//////////////////////////////////////////////////////////////////////////
wxDECLARE_EVENT(KEVT_PLUGIN_TOGGLED, KPluginEvent);
wxDECLARE_EVENT(KEVT_PLUGIN_CHANGED, KPluginEvent);

wxDECLARE_EVENT(KEVT_PLUGINS_REORDERED, KPluginEvent);
