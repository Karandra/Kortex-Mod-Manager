#include "stdafx.h"
#include "ProfileEvent.h"
#include "IGameProfile.h"

namespace Kortex
{
	wxString ProfileEvent::GetProfileID() const
	{
		return m_Profile ? m_Profile->GetID() : GetString();
	}
}
