#include "stdafx.h"
#include "SaveEvent.h"
#include <Kortex/SaveManager.hpp>

namespace Kortex::SaveManager
{
	wxString SaveEvent::GetSaveName() const
	{
		return m_Save ? m_Save->GetFileItem().GetName() : wxNotifyEvent::GetString();
	}
}

namespace Kortex::Events
{
	wxDEFINE_EVENT(SaveRemoving, SaveManager::SaveEvent);
	wxDEFINE_EVENT(SaveRemoved, SaveManager::SaveEvent);

	wxDEFINE_EVENT(SaveChanged, SaveManager::SaveEvent);
}
