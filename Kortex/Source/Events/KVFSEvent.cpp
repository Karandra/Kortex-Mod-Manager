#include "stdafx.h"
#include "KVFSEvent.h"

wxDEFINE_EVENT(KEVT_BROADCAST_VFS_TOGGLED, KVFSEvent);

KVFSEvent::KVFSEvent(wxEventType type)
	:KBroadcastEvent(type)
{
}
KVFSEvent::KVFSEvent(bool activated)
	:KBroadcastEvent(KEVT_BROADCAST_VFS_TOGGLED), m_Activated(activated)
{
}
KVFSEvent::~KVFSEvent()
{
}
KVFSEvent* KVFSEvent::Clone() const
{
	return new KVFSEvent(*this);
}
