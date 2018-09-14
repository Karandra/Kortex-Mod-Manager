#pragma once
#include "stdafx.h"
#include <KxFramework/KxBroadcastEvent.h>

class KEvent: public KxBroadcastEvent
{
	public:
		KEvent(wxEventType type = wxEVT_NULL)
			:KxBroadcastEvent(type)
		{
		}
};
