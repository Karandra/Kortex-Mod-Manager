#pragma once
#include "stdafx.h"

namespace Kortex::Utility
{
	class QuickThread: public wxThread
	{
		public:
			KxEVENT_MEMBER(wxNotifyEvent, ThreadEnd);

		private:
			std::function<void()> m_Entry;
			wxEvtHandler* m_EventHandler = nullptr;
			bool m_EndEventSent = false;

		private:
			virtual ExitCode Entry() override;
			virtual void OnExit() override;
			wxThreadError Delete(ExitCode*, wxThreadWait) = delete;

		public:
			template<class TEntryPoint>
			QuickThread(TEntryPoint entryPoint, wxEvtHandler* eventHandler = nullptr)
				:wxThread(wxTHREAD_DETACHED), m_Entry(std::move(entryPoint)), m_EventHandler(eventHandler)
			{
			}
			virtual ~QuickThread() = default;

		public:
			bool Run();
			bool Destroy();
			bool TestDestroy() override;

			bool CanSendEvents() const
			{
				return m_EventHandler != nullptr;
			}
			void QueueEvent(std::unique_ptr<wxEvent> event);
	};
}