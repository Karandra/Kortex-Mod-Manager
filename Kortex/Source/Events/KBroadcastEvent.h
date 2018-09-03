#pragma once
#include "stdafx.h"
#include "KEvent.h"
class KWorkspace;

class KBroadcastEvent: public KEvent
{
	private:
		wxString m_TargetWorkspaceID;

	public:
		KBroadcastEvent(wxEventType type = wxEVT_NULL)
			:KEvent(type)
		{
		}
		KBroadcastEvent* Clone() const override
		{
			return new KBroadcastEvent(*this);
		}

	public:
		const wxString& GetTargetWorkspaceID() const
		{
			return m_TargetWorkspaceID;
		}
		void SetTargetWorkspaceID(const wxString& id)
		{
			m_TargetWorkspaceID = id;
		}
		bool IsTargetWorkspace(const KWorkspace* workspace) const;
};
