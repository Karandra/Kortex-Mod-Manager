#pragma once
#include "stdafx.h"
#include "IEvent.h"

namespace Kortex
{
	class IVirtualFileSystem;

	class VFSEvent: public IEvent
	{
		private:
			bool m_Activated = false;
			IVirtualFileSystem* m_VFS = nullptr;

		public:
			VFSEvent(wxEventType type, bool activated)
				:IEvent(type), m_Activated(activated)
			{
			}
			VFSEvent(wxEventType type, IVirtualFileSystem& vfs, bool activated)
				:IEvent(type), m_VFS(&vfs), m_Activated(activated)
			{
			}

			VFSEvent* Clone() const override
			{
				return new VFSEvent(*this);
			}

		public:
			IVirtualFileSystem* GetFileSystem() const
			{
				return m_VFS;
			}
			bool IsActivated() const
			{
				return m_Activated;
			}
	};
}

namespace Kortex::Events
{
	wxDECLARE_EVENT(SingleVFSToggled, VFSEvent);
	wxDECLARE_EVENT(MainVFSToggled, VFSEvent);
}
