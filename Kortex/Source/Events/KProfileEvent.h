#pragma once
#include "stdafx.h"
#include "KEvent.h"
class KProfile;

class KProfileEvent: public KEvent
{
	private:
		KProfile* m_Profile = NULL;

	public:
		KProfileEvent(wxEventType type)
			:KEvent(type)
		{
		}
		KProfileEvent(wxEventType type, KProfile& profile)
			:KEvent(type), m_Profile(&profile)
		{
		}
		KProfileEvent(wxEventType type, const wxString& id)
			:KEvent(type)
		{
			wxNotifyEvent::SetString(id);
		}

		KProfileEvent* Clone() const override
		{
			return new KProfileEvent(*this);
		}

	public:
		bool HasProfile() const
		{
			return m_Profile != NULL;
		}
		KProfile* GetProfile() const
		{
			return m_Profile;
		}
		wxString GetProfileID() const;
};

//////////////////////////////////////////////////////////////////////////
wxDECLARE_EVENT(KEVT_PROFILE_UPDATE_LIST, KProfileEvent);

wxDECLARE_EVENT(KEVT_PROFILE_ADDING, KProfileEvent);
wxDECLARE_EVENT(KEVT_PROFILE_ADDED, KProfileEvent);

wxDECLARE_EVENT(KEVT_PROFILE_REMOVING, KProfileEvent);
wxDECLARE_EVENT(KEVT_PROFILE_REMOVED, KProfileEvent);

wxDECLARE_EVENT(KEVT_PROFILE_CHANGED, KProfileEvent);

wxDECLARE_EVENT(KEVT_PROFILE_SELECTED, KProfileEvent);
