#pragma once
#include "stdafx.h"
#include "Application/BroadcastProcessor.h"

namespace Kortex
{
	class IGameSave;
}

namespace Kortex
{
	class SaveEvent: public BroadcastEvent
	{
		public:
			using RefVector = std::vector<IGameSave*>;

		public:
			KxEVENT_MEMBER(SaveEvent, Removing);
			KxEVENT_MEMBER(SaveEvent, Removed);
			KxEVENT_MEMBER(SaveEvent, Changed);

		private:
			RefVector m_SavesArray;
			IGameSave* m_Save = nullptr;

		public:
			SaveEvent(IGameSave& save)
				:m_Save(&save)
			{
			}
			SaveEvent(RefVector saves)
				:m_SavesArray(saves)
			{
			}
			SaveEvent(const wxString& name)
			{
				SetString(name);
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
