#include "stdafx.h"
#include <Kortex/Application.hpp>
#include "Events/LogEvent.h"


namespace Kortex
{
	LogEvent::LogEvent(const wxString& message, LogLevel level, wxWindow* window)
		:IEvent(Events::Log), m_Level(level), m_Window(window)
	{
		SetString(message);
	}
	LogEvent::~LogEvent()
	{
		if (!m_EventSent)
		{
			Send();
		}
	}
	LogEvent* LogEvent::Clone() const
	{
		LogEvent* event = new LogEvent(*this);
		event->SetString(GetMessage().Clone());

		return event;
	}

	bool LogEvent::Send()
	{
		if (!m_EventSent)
		{
			m_EventSent = true;
			if (wxThread::IsMain())
			{
				return IEvent::SendEvent(*this);
			}
			else
			{
				IEvent::QueueEvent(Clone());
			}
		}
		return false;
	}
}

namespace Kortex::Events
{
	wxDEFINE_EVENT(Log, Kortex::LogEvent);
}
