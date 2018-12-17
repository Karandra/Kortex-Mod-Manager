#pragma once
#include "stdafx.h"
#include "IEvent.h"

namespace Kortex
{
	class IGameProfile;
}

namespace Kortex::GameInstance
{
	class ProfileEvent: public IEvent
	{
		private:
			IGameProfile* m_Profile = nullptr;

		public:
			ProfileEvent(wxEventType type)
				:IEvent(type)
			{
			}
			ProfileEvent(wxEventType type, IGameProfile& profile)
				:IEvent(type), m_Profile(&profile)
			{
			}
			ProfileEvent(wxEventType type, const wxString& id)
				:IEvent(type)
			{
				wxNotifyEvent::SetString(id);
			}

			ProfileEvent* Clone() const override
			{
				return new ProfileEvent(*this);
			}

		public:
			bool HasProfile() const
			{
				return m_Profile != nullptr;
			}
			IGameProfile* GetProfile() const
			{
				return m_Profile;
			}
			wxString GetProfileID() const;
	};
}

namespace Kortex::Events
{
	wxDECLARE_EVENT(ProfileRefreshList, GameInstance::ProfileEvent);

	wxDECLARE_EVENT(ProfileAdding, GameInstance::ProfileEvent);
	wxDECLARE_EVENT(ProfileAdded, GameInstance::ProfileEvent);

	wxDECLARE_EVENT(ProfileRemoving, GameInstance::ProfileEvent);
	wxDECLARE_EVENT(ProfileRemoved, GameInstance::ProfileEvent);

	wxDECLARE_EVENT(ProfileChanged, GameInstance::ProfileEvent);
	wxDECLARE_EVENT(ProfileSelected, GameInstance::ProfileEvent);
}
