#pragma once
#include "stdafx.h"
#include "KEvent.h"

class KVFSEvent: public KEvent
{
	private:
		bool m_Activated = false;

	public:
		KVFSEvent(wxEventType type = wxEVT_NULL)
			:KEvent(type)
		{
		}
		KVFSEvent(bool activated);

		KVFSEvent* Clone() const override
		{
			return new KVFSEvent(*this);
		}

	public:
		bool IsActivated() const
		{
			return m_Activated;
		}
};

//////////////////////////////////////////////////////////////////////////
wxDECLARE_EVENT(KEVT_VFS_TOGGLED, KVFSEvent);
