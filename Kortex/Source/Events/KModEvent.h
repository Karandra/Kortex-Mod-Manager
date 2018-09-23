#pragma once
#include "stdafx.h"
#include "KEvent.h"
class KModEntry;

class KModEvent: public KEvent
{
	private:
		KModEntry* m_Mod = NULL;
		std::vector<KModEntry*> m_ModsArray;

	public:
		KModEvent(wxEventType type = wxEVT_NULL)
			:KEvent(type)
		{
		}
		KModEvent(wxEventType type, KModEntry& modEntry)
			:KEvent(type), m_Mod(&modEntry)
		{
		}
		KModEvent(wxEventType type, std::vector<KModEntry*> modsArray)
			:KEvent(type), m_ModsArray(modsArray)
		{
		}
		KModEvent(wxEventType type, const wxString& id)
			:KEvent(type)
		{
			wxNotifyEvent::SetString(id);
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

		bool HasModArray() const
		{
			return !m_ModsArray.empty();
		}
		const std::vector<KModEntry*>& GetModsArray() const
		{
			return m_ModsArray;
		}

};

//////////////////////////////////////////////////////////////////////////
wxDECLARE_EVENT(KEVT_MOD_INSTALLING, KModEvent);
wxDECLARE_EVENT(KEVT_MOD_INSTALLED, KModEvent);

wxDECLARE_EVENT(KEVT_MOD_UNINSTALLING, KModEvent);
wxDECLARE_EVENT(KEVT_MOD_UNINSTALLED, KModEvent);

wxDECLARE_EVENT(KEVT_MOD_TOGGLED, KModEvent);
wxDECLARE_EVENT(KEVT_MOD_CHANGED, KModEvent);
wxDECLARE_EVENT(KEVT_MOD_FILES_CHANGED, KModEvent);

wxDECLARE_EVENT(KEVT_MODS_REORDERED, KModEvent);
