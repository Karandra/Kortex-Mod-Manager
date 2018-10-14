#include "stdafx.h"
#include "KProfileEvent.h"
#include "Profile/KProfile.h"

wxString KProfileEvent::GetProfileID() const
{
	return m_Profile ? m_Profile->GetID() : wxNotifyEvent::GetString();
}

//////////////////////////////////////////////////////////////////////////
wxDEFINE_EVENT(KEVT_PROFILE_UPDATE_LIST, KProfileEvent);

wxDEFINE_EVENT(KEVT_PROFILE_ADDING, KProfileEvent);
wxDEFINE_EVENT(KEVT_PROFILE_ADDED, KProfileEvent);

wxDEFINE_EVENT(KEVT_PROFILE_REMOVING, KProfileEvent);
wxDEFINE_EVENT(KEVT_PROFILE_REMOVED, KProfileEvent);

wxDEFINE_EVENT(KEVT_PROFILE_CHANGED, KProfileEvent);
wxDEFINE_EVENT(KEVT_PROFILE_SELECTED, KProfileEvent);

wxDEFINE_EVENT(KiEVT_PROFILE_SELECTED, KProfileEvent);
