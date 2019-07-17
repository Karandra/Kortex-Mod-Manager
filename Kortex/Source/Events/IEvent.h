#pragma once
#include "stdafx.h"
#include <KxFramework/KxEvent.h>
#include <KxFramework/KxBroadcastEvent.h>

namespace Kortex
{
	class IEvent: public KxBroadcastEvent
	{
		public:
			IEvent(wxEventType type = wxEVT_NULL)
				:KxBroadcastEvent(type)
			{
			}
	};
}
