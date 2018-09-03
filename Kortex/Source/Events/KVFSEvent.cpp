#include "stdafx.h"
#include "KVFSEvent.h"

KVFSEvent::KVFSEvent(bool activated)
	:KEvent(KEVT_VFS_TOGGLED), m_Activated(activated)
{
}

//////////////////////////////////////////////////////////////////////////
wxDEFINE_EVENT(KEVT_VFS_TOGGLED, KVFSEvent);
