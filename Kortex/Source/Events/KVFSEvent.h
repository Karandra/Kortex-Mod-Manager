#pragma once
#include "stdafx.h"
#include "KBroadcastEvent.h"

class KVFSEvent: public KBroadcastEvent
{
	private:
		bool m_Activated = false;

	public:
		KVFSEvent(wxEventType type = wxEVT_NULL);
		KVFSEvent(bool isEnabled);
		virtual ~KVFSEvent();
		KVFSEvent* Clone() const override;

	public:
		bool IsActivated() const
		{
			return m_Activated;
		}
};

wxDECLARE_EVENT(KEVT_BROADCAST_VFS_TOGGLED, KVFSEvent);
