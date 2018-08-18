#pragma once
#include "stdafx.h"

wxDECLARE_EVENT(KEVT_QUICK_THREAD_END, wxNotifyEvent);

class KQuickThread: public wxThread
{
	public:
		typedef std::function<void(KQuickThread&)> EntryType;

	private:
		EntryType m_Entry;
		wxEvtHandler* m_EventHandler = NULL;
		bool m_EndEventSent = false;

	private:
		virtual ExitCode Entry() override;
		virtual void OnExit() override;
		wxThreadError Delete(ExitCode*, wxThreadWait) = delete;

	public:
		KQuickThread(const EntryType& entryPoint, wxEvtHandler* eventHandler = NULL);
		virtual ~KQuickThread();

	public:
		bool Run();
		bool Destroy();
		bool TestDestroy();

		bool CanSendEvents() const
		{
			return m_EventHandler != NULL;
		}
		void QueueEvent(wxEvent* event);
		void QueueEvent(const wxEvent& event);
};
