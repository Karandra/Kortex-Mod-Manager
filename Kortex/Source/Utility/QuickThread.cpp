#include "stdafx.h"
#include "QuickThread.h"
#include <Kortex/Application.hpp>
#include "Utility/Log.h"

namespace Kortex::Utility
{
	wxThread::ExitCode QuickThread::Entry()
	{
		const size_t threadID = static_cast<size_t>(GetId());

		wxLog::SetThreadActiveTarget(&Kortex::IApplication::GetInstance()->GetLogger());
		Log::LogMessage("Thread 0x%1 (ID: %2) started", this, threadID);

		m_Entry();
		if (!m_EndEventSent)
		{
			OnExit();
		}

		Log::LogMessage("Thread 0x%1 (ID: %2) exited", this, threadID);
		return nullptr;
	}
	void QuickThread::OnExit()
	{
		if (CanSendEvents() && !m_EndEventSent)
		{
			m_EndEventSent = true;
			QueueEvent(std::make_unique<wxNotifyEvent>(EvtThreadEnd, GetId()));
		}
	}

	bool QuickThread::Run()
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
	bool QuickThread::Destroy()
	{
		return wxThread::Delete() == wxTHREAD_NO_ERROR;
	}
	bool QuickThread::TestDestroy()
	{
		return wxThread::TestDestroy();
	}

	void QuickThread::QueueEvent(std::unique_ptr<wxEvent> event)
	{
		if (m_EventHandler)
		{
			if (event->GetEventObject() == nullptr)
			{
				event->SetEventObject(m_EventHandler);
			}
			m_EventHandler->QueueEvent(event.release());
		}
	}
}
