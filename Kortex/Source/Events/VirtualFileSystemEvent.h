#pragma once
#include "stdafx.h"
#include "IEvent.h"

namespace Kortex
{
	class VirtualFileSystemEvent: public IEvent
	{
		private:
			bool m_Activated = false;

		public:
			VirtualFileSystemEvent(wxEventType type = wxEVT_NULL)
				:IEvent(type)
			{
			}
			VirtualFileSystemEvent(wxEventType type, bool activated)
				:IEvent(type), m_Activated(activated)
			{
			}

			VirtualFileSystemEvent* Clone() const override
			{
				return new VirtualFileSystemEvent(*this);
			}

		public:
			bool IsActivated() const
			{
				return m_Activated;
			}
	};
}

namespace Kortex::Events
{
	wxDECLARE_EVENT(VirtualFileSystemToggled, VirtualFileSystemEvent);
}
