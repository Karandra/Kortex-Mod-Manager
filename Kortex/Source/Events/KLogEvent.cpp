#include "stdafx.h"
#include "KApp.h"
#include "Events/KLogEvent.h"

wxDEFINE_EVENT(KEVT_LOG, KLogEvent);

KLogEvent::KLogEvent(const wxString& message, KLogLevel level, wxWindow* window)
	:KEvent(KEVT_LOG), m_Level(level), m_Window(window)
{
	SetString(message);
}
KLogEvent::~KLogEvent()
{
	if (!m_EventSent)
	{
		Send();
	}
}
KLogEvent* KLogEvent::Clone() const
{
	KLogEvent* event = new KLogEvent(*this);
	event->SetString(GetMessage().Clone());

	return event;
}

bool KLogEvent::Send()
{
	if (!m_EventSent)
	{
		m_EventSent = true;
		if (wxThread::IsMain())
		{
			return KEvent::SendEvent(*this);
		}
		else
		{
			KEvent::QueueEvent(Clone());
		}
	}
	return false;
}
