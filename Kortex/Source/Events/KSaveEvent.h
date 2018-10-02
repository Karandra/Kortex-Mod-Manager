#pragma once
#include "stdafx.h"
#include "KEvent.h"
class KSaveFile;

class KSaveEvent: public KEvent
{
	public:
		using RefVector = std::vector<KSaveFile*>;

	private:
		KSaveFile* m_Save = NULL;
		RefVector m_SavesArray;

	public:
		KSaveEvent(wxEventType type = wxEVT_NULL)
			:KEvent(type)
		{
		}
		KSaveEvent(wxEventType type, KSaveFile& modEntry)
			:KEvent(type), m_Save(&modEntry)
		{
		}
		KSaveEvent(wxEventType type, RefVector modsArray)
			:KEvent(type), m_SavesArray(modsArray)
		{
		}
		KSaveEvent(wxEventType type, const wxString& name)
			:KEvent(type)
		{
			wxNotifyEvent::SetString(name);
		}

		KSaveEvent* Clone() const override
		{
			return new KSaveEvent(*this);
		}

	public:
		bool HasSave() const
		{
			return m_Save != NULL;
		}
		KSaveFile* GetSave() const
		{
			return m_Save;
		}
		wxString GetSaveName() const;

		bool HasSavesArray() const
		{
			return !m_SavesArray.empty();
		}
		const RefVector& GetSavesArray() const
		{
			return m_SavesArray;
		}
};

//////////////////////////////////////////////////////////////////////////
wxDECLARE_EVENT(KEVT_SAVE_REMOVING, KSaveEvent);
wxDECLARE_EVENT(KEVT_SAVE_REMOVED, KSaveEvent);

wxDECLARE_EVENT(KEVT_SAVE_CHANGED, KSaveEvent);
