#pragma once
#include "stdafx.h"
#include "Application/BroadcastProcessor.h"

namespace Kortex
{
	class LogEvent: public BroadcastEvent
	{
		public:
			KxEVENT_MEMBER(LogEvent, Info);
			KxEVENT_MEMBER(LogEvent, Warning);
			KxEVENT_MEMBER(LogEvent, Error);
			KxEVENT_MEMBER(LogEvent, Critical);

		private:
			wxWindow* m_Window = nullptr;
			bool m_EventSent = false;

		public:
			LogEvent() = default;
			LogEvent(const wxString& message, wxWindow* window = nullptr)
				:m_Window(window)
			{
				SetString(message);
			}

		public:
			LogEvent* Clone() const override
			{
				return new LogEvent(*this);
			}

			KxEventTag<LogEvent> GetLevel() const
			{
				return GetEventType();
			}
			bool IsCritical() const
			{
				return GetLevel() == EvtCritical;
			}

			wxWindow* GetWindow() const
			{
				return m_Window;
			}
			void SetWindow(wxWindow* window)
			{
				m_Window = window;
			}
	};
}
