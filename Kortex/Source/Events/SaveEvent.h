#pragma once
#include "stdafx.h"
#include "IEvent.h"

namespace Kortex
{
	class IGameSave;
}

namespace Kortex::SaveManager
{
	class SaveEvent: public IEvent
	{
		public:
			using RefVector = std::vector<IGameSave*>;

		private:
			IGameSave* m_Save = nullptr;
			RefVector m_SavesArray;

		public:
			SaveEvent(wxEventType type = wxEVT_NULL)
				:IEvent(type)
			{
			}
			SaveEvent(wxEventType type, IGameSave& save)
				:IEvent(type), m_Save(&save)
			{
			}
			SaveEvent(wxEventType type, RefVector saves)
				:IEvent(type), m_SavesArray(saves)
			{
			}
			SaveEvent(wxEventType type, const wxString& name)
				:IEvent(type)
			{
				wxNotifyEvent::SetString(name);
			}

			SaveEvent* Clone() const override
			{
				return new SaveEvent(*this);
			}

		public:
			bool HasSave() const
			{
				return m_Save != nullptr;
			}
			IGameSave* GetSave() const
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
}

namespace Kortex::Events
{
	wxDECLARE_EVENT(SaveRemoving, SaveManager::SaveEvent);
	wxDECLARE_EVENT(SaveRemoved, SaveManager::SaveEvent);

	wxDECLARE_EVENT(SaveChanged, SaveManager::SaveEvent);
}
