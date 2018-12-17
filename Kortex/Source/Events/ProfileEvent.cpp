#include "stdafx.h"
#include "ProfileEvent.h"
#include "GameInstance/IGameProfile.h"

namespace Kortex::GameInstance
{
	wxString ProfileEvent::GetProfileID() const
	{
		return m_Profile ? m_Profile->GetID() : wxNotifyEvent::GetString();
	}
}

namespace Kortex::Events
{
	wxDEFINE_EVENT(ProfileRefreshList, GameInstance::ProfileEvent);

	wxDEFINE_EVENT(ProfileAdding, GameInstance::ProfileEvent);
	wxDEFINE_EVENT(ProfileAdded, GameInstance::ProfileEvent);

	wxDEFINE_EVENT(ProfileRemoving, GameInstance::ProfileEvent);
	wxDEFINE_EVENT(ProfileRemoved, GameInstance::ProfileEvent);

	wxDEFINE_EVENT(ProfileChanged, GameInstance::ProfileEvent);
	wxDEFINE_EVENT(ProfileSelected, GameInstance::ProfileEvent);

	wxDEFINE_EVENT(KiEVT_PROFILE_SELECTED, GameInstance::ProfileEvent);
}
