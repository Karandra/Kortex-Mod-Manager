#pragma once
#include "stdafx.h"
#include "KEvent.h"
class KModEntry;

class KModEvent: public KEvent
{
	private:
		KModEntry* m_Mod = NULL;

	public:
		KModEvent(wxEventType type = wxEVT_NULL)
			:KEvent(type)
		{
		}
		KModEvent(wxEventType type, KModEntry& modEntry)
			:KEvent(type), m_Mod(&modEntry)
		{
		}
		KModEvent(wxEventType type, const wxString& modID)
			:KEvent(type)
		{
			wxNotifyEvent::SetString(modID);
		}

		KModEvent* Clone() const override
		{
			return new KModEvent(*this);
		}

	public:
		bool HasMod() const
		{
			return m_Mod != NULL;
		}
		KModEntry* GetMod() const
		{
			return m_Mod;
		}

		wxString GetModID() const;
};

//////////////////////////////////////////////////////////////////////////
wxDECLARE_EVENT(KEVT_MOD_INSTALLING, KModEvent);
wxDECLARE_EVENT(KEVT_MOD_INSTALLED, KModEvent);

wxDECLARE_EVENT(KEVT_MOD_UNINSTALLING, KModEvent);
wxDECLARE_EVENT(KEVT_MOD_UNINSTALLED, KModEvent);

wxDECLARE_EVENT(KEVT_MOD_CHNAGED, KModEvent);
wxDECLARE_EVENT(KEVT_MOD_FILES_CHNAGED, KModEvent);
