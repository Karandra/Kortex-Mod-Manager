#pragma once
#include "stdafx.h"
#include "IEvent.h"

namespace Kortex
{
	class IGamePlugin;
}

namespace Kortex::PluginManager
{
	class PluginEvent: public IEvent
	{
		public:
			using RefVector = std::vector<IGamePlugin*>;

		private:
			IGamePlugin* m_Plugin = nullptr;
			RefVector m_PluginsVector;

		public:
			PluginEvent(wxEventType type = wxEVT_NULL)
				:IEvent(type)
			{
			}
			PluginEvent(wxEventType type, IGamePlugin& plugin)
				:IEvent(type), m_Plugin(&plugin)
			{
			}
			PluginEvent(wxEventType type, RefVector plugins)
				:IEvent(type), m_PluginsVector(plugins)
			{
			}
			PluginEvent(wxEventType type, const wxString& name)
				:IEvent(type)
			{
				wxNotifyEvent::SetString(name);
			}

			PluginEvent* Clone() const override
			{
				return new PluginEvent(*this);
			}

		public:
			bool HasPlugin() const
			{
				return m_Plugin != nullptr;
			}
			IGamePlugin* GetPlugin() const
			{
				return m_Plugin;
			}
			wxString GetPluginName() const;

			bool HasPluginsArray() const
			{
				return !m_PluginsVector.empty();
			}
			const RefVector& GetPluginsArray() const
			{
				return m_PluginsVector;
			}

	};
}

namespace Kortex::Events
{
	wxDECLARE_EVENT(PluginToggled, PluginManager::PluginEvent);
	wxDECLARE_EVENT(PluginChanged, PluginManager::PluginEvent);

	wxDECLARE_EVENT(PluginsReordered, PluginManager::PluginEvent);
}
