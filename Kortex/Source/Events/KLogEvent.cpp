#include "stdafx.h"
#include "KApp.h"
#include "Events/KLogEvent.h"

wxDEFINE_EVENT(KEVT_LOG, KLogEvent);

KLogEvent::KLogEvent(const wxString& message, KLogLevel level, wxWindow* window)
	:wxNotifyEvent(KEVT_LOG), m_Level(level), m_Window(window)
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

void KLogEvent::Abandon()
{
	m_EventSent = true;
}
void KLogEvent::Send()
{
	if (!m_EventSent)
	{
		m_EventSent = true;
		if (wxThread::IsMain())
		{
			KApp::Get().ProcessEvent(*this);
		}
		else
		{
			KApp::Get().QueueEvent(Clone());
		}
	}
}
