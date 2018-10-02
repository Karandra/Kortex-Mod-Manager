#include "stdafx.h"
#include "KSaveEvent.h"
#include "SaveManager/KSaveFile.h"

wxString KSaveEvent::GetSaveName() const
{
	return m_Save ? m_Save->GetFileInfo().GetName() : wxNotifyEvent::GetString();
}

//////////////////////////////////////////////////////////////////////////
wxDEFINE_EVENT(KEVT_SAVE_REMOVING, KSaveEvent);
wxDEFINE_EVENT(KEVT_SAVE_REMOVED, KSaveEvent);

wxDEFINE_EVENT(KEVT_SAVE_CHANGED, KSaveEvent);
