#pragma once
#include "stdafx.h"
#include "KEvent.h"
class KModList;

class KModListEvent: public KEvent
{
	private:
		KModList* m_ModList = NULL;

	public:
		KModListEvent(wxEventType type)
			:KEvent(type)
		{
		}
		KModListEvent(wxEventType type, KModList& modList)
			:KEvent(type), m_ModList(&modList)
		{
		}
		KModListEvent(wxEventType type, const wxString& id)
			:KEvent(type)
		{
			wxNotifyEvent::SetString(id);
		}

		KModListEvent* Clone() const override
		{
			return new KModListEvent(*this);
		}

	public:
		bool HasModList() const
		{
			return m_ModList != NULL;
		}
		KModList* GetModList() const
		{
			return m_ModList;
		}
		wxString GetModListID() const;
};

//////////////////////////////////////////////////////////////////////////
wxDECLARE_EVENT(KEVT_MODLIST_ADDING, KModListEvent);
wxDECLARE_EVENT(KEVT_MODLIST_ADDED, KModListEvent);

wxDECLARE_EVENT(KEVT_MODLIST_REMOVING, KModListEvent);
wxDECLARE_EVENT(KEVT_MODLIST_REMOVED, KModListEvent);

wxDECLARE_EVENT(KEVT_MODLIST_CHANGED, KModListEvent);
wxDECLARE_EVENT(KEVT_MODLIST_SELECTED, KModListEvent);
