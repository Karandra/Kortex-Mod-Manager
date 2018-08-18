#pragma once
#include "stdafx.h"

enum KLogLevel
{
	KLOG_INFO,
	KLOG_WARNING,
	KLOG_ERROR,
	KLOG_CRITICAL,
};
class KLogEvent: public wxNotifyEvent
{
	private:
		KLogLevel m_Level = KLOG_INFO;
		wxWindow* m_Window = NULL;
		bool m_EventSent = false;

	public:
		KLogEvent(const wxString& message, KLogLevel level, wxWindow* window = NULL);
		virtual ~KLogEvent();
		KLogEvent* Clone() const override;

	public:
		void Send();
		void Abandon();

		bool IsCritical() const
		{
			return m_Level == KLOG_CRITICAL;
		}
		KLogLevel GetLevel() const
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

wxDECLARE_EVENT(KEVT_LOG, KLogEvent);

