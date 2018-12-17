#pragma once
#include "stdafx.h"
#include "IEvent.h"

namespace Kortex
{
	enum class LogLevel
	{
		Info,
		Warning,
		Error,
		Critical,
	};

	class LogEvent: public IEvent
	{
		private:
			LogLevel m_Level = LogLevel::Info;
			wxWindow* m_Window = nullptr;
			bool m_EventSent = false;

		public:
			LogEvent(const wxString& message, LogLevel level, wxWindow* window = nullptr);
			virtual ~LogEvent();
			LogEvent* Clone() const override;

		public:
			bool Send();

			bool IsCritical() const
			{
				return m_Level == LogLevel::Critical;
			}
			LogLevel GetLevel() const
			{
				return m_Level;
			}
			wxString GetMessage() const
			{
				return GetString();
			}
			wxWindow* GetWindow() const
			{
				return m_Window;
			}
	};
}

namespace Kortex::Events
{
	wxDECLARE_EVENT(Log, Kortex::LogEvent);
}
