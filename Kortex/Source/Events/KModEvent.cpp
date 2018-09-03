#include "stdafx.h"
#include "KModEvent.h"
#include "ModManager/KModEntry.h"

wxString KModEvent::GetModID() const
{
	return m_Mod ? m_Mod->GetID() : wxNotifyEvent::GetString();
}

//////////////////////////////////////////////////////////////////////////
wxDEFINE_EVENT(KEVT_MOD_INSTALLING, KModEvent);
wxDEFINE_EVENT(KEVT_MOD_INSTALLED, KModEvent);

wxDEFINE_EVENT(KEVT_MOD_UNINSTALLING, KModEvent);
wxDEFINE_EVENT(KEVT_MOD_UNINSTALLED, KModEvent);

wxDEFINE_EVENT(KEVT_MOD_CHNAGED, KModEvent);
wxDEFINE_EVENT(KEVT_MOD_FILES_CHNAGED, KModEvent);
