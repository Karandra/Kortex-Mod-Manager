#pragma once
#include "stdafx.h"
#include <Kx/Core/EventSystem/BroadcastProcessor.h>

namespace Kortex
{
	class BroadcastReciever;
}

namespace Kortex
{
	class BroadcastProcessor: public KxBroadcastProcessor
	{
		public:
			static BroadcastProcessor& Get();

		public:
			BroadcastProcessor() = default;

		public:
			bool AddReciever(BroadcastReciever& reciever);
			bool RemoveReciever(BroadcastReciever& reciever);
	};
}

namespace Kortex
{
	class BroadcastReciever: public KxBroadcastReciever
	{
		public:
			BroadcastReciever(BroadcastProcessor& processor)
				:KxBroadcastReciever(processor)
			{
			}
			BroadcastReciever()
				:KxBroadcastReciever(BroadcastProcessor::Get())
			{
			}
	
		public:
			BroadcastProcessor& GetProcessor()
			{
				return static_cast<BroadcastProcessor&>(KxBroadcastReciever::GetProcessor());
			}
			const BroadcastProcessor& GetProcessor() const
			{
				return static_cast<const BroadcastProcessor&>(KxBroadcastReciever::GetProcessor());
			}
	};
}

namespace Kortex
{
	class BroadcastEvent: public wxNotifyEvent
	{
		public:
			BroadcastEvent()
				:wxNotifyEvent(KxEvent::EvtNull)
			{
			}

		public:
			BroadcastEvent* Clone() const override
			{
				return new BroadcastEvent(*this);
			}
	};
}
