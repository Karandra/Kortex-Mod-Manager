#pragma once
#include "stdafx.h"

class KBroadcastEvent: public wxNotifyEvent
{
	private:
		wxString m_TargetWorkspaceID;

	public:
		KBroadcastEvent(wxEventType type = wxEVT_NULL);
		virtual ~KBroadcastEvent();
		KBroadcastEvent* Clone() const override;

	public:
		const wxString& GetTargetWorkspaceID() const
		{
			return m_TargetWorkspaceID;
		}
		void SetTargetWorkspaceID(const wxString& id)
		{
			m_TargetWorkspaceID = id;
		}
};
