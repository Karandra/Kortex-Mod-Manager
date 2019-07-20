#pragma once
#include "stdafx.h"
#include "Application/BroadcastProcessor.h"

namespace Kortex
{
	class IGameProfile;
}

namespace Kortex
{
	class ProfileEvent: public BroadcastEvent
	{
		public:
			KxEVENT_MEMBER(ProfileEvent, Adding);
			KxEVENT_MEMBER(ProfileEvent, Added);

			KxEVENT_MEMBER(ProfileEvent, Removing);
			KxEVENT_MEMBER(ProfileEvent, Removed);

			KxEVENT_MEMBER(ProfileEvent, Changed);
			KxEVENT_MEMBER(ProfileEvent, Selected);

			KxEVENT_MEMBER(ProfileEvent, RefreshList);

		private:
			IGameProfile* m_Profile = nullptr;

		public:
			ProfileEvent() = default;
			ProfileEvent(IGameProfile& profile)
				:m_Profile(&profile)
			{
			}
			ProfileEvent(const wxString& id)
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
