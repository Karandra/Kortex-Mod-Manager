#include "stdafx.h"
#include "KQuickThread.h"
#include <Kortex/Application.hpp>
#include "Utility/Log.h"

using namespace Kortex;
wxDEFINE_EVENT(KEVT_QUICK_THREAD_END, wxNotifyEvent);

wxThread::ExitCode KQuickThread::Entry()
{
	wxLog::SetThreadActiveTarget(&Kortex::IApplication::GetInstance()->GetLogger());
	Utility::Log::LogMessage("Thread 0x%1 (ID: %2) started", this, (size_t)GetId());

	m_Entry(*this);
	if (!m_EndEventSent)
	{
		OnExit();
	}

	Utility::Log::LogMessage("Thread 0x%1 (ID: %2) exited", this, (size_t)GetId());
	return 0;
}
void KQuickThread::OnExit()
{
	if (CanSendEvents() && !m_EndEventSent)
	{
		m_EndEventSent = true;
		wxNotifyEvent* event = new wxNotifyEvent(KEVT_QUICK_THREAD_END, GetId());
		QueueEvent(event);
	}
}

KQuickThread::KQuickThread(const EntryType& entryPoint, wxEvtHandler* eventHandler)
	:wxThread(wxTHREAD_DETACHED), m_Entry(entryPoint), m_EventHandler(eventHandler)
{
}
KQuickThread::~KQuickThread()
{
}

bool KQuickThread::Run()
{
	if (wxThread::Run() == wxTHREAD_NO_ERROR)
	{
		return true;
	}
	else
	{
		delete this;
		return false;
	}
}
bool KQuickThread::Destroy()
{
	return wxThread::Delete() == wxTHREAD_NO_ERROR;
}
bool KQuickThread::TestDestroy()
{
	return wxThread::TestDestroy();
}

void KQuickThread::QueueEvent(wxEvent* event)
{
	if (m_EventHandler)
	{
		if (event->GetEventObject() == nullptr)
		{
			event->SetEventObject(m_EventHandler);
		}
		m_EventHandler->QueueEvent(event);
	}
	else
	{
		delete event;
	}
}
void KQuickThread::QueueEvent(const wxEvent& event)
{
	if (m_EventHandler)
	{
		QueueEvent(event.Clone());
	}
}
