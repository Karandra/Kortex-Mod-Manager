#include "stdafx.h"
#include "KModListEvent.h"
#include "KModListEventInternal.h"
#include "ModManager/KModListManager.h"

wxString KModListEvent::GetModListID() const
{
	return m_ModList ? m_ModList->GetID() : wxNotifyEvent::GetString();
}

//////////////////////////////////////////////////////////////////////////
wxDEFINE_EVENT(KEVT_MODLIST_ADDING, KModListEvent);
wxDEFINE_EVENT(KEVT_MODLIST_ADDED, KModListEvent);

wxDEFINE_EVENT(KEVT_MODLIST_REMOVING, KModListEvent);
wxDEFINE_EVENT(KEVT_MODLIST_REMOVED, KModListEvent);

wxDEFINE_EVENT(KEVT_MODLIST_CHANGED, KModListEvent);
wxDEFINE_EVENT(KEVT_MODLIST_SELECTED, KModListEvent);

wxDEFINE_EVENT(KEVT_MODLIST_INT_SELECTED, KModListEvent);
