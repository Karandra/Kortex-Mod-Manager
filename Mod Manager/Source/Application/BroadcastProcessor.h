#pragma once
#include "Framework.hpp"
#include <kxf/EventSystem/EventBroadcastProcessor.h>

namespace Kortex
{
	class BroadcastReciever;
}

namespace Kortex
{
	class BroadcastProcessor: public kxf::EventBroadcastProcessor
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
	class BroadcastReciever: public kxf::EventBroadcastReciever
	{
		public:
			BroadcastReciever()
				:BroadcastReciever(BroadcastProcessor::Get())
			{
			}
			BroadcastReciever(BroadcastProcessor& processor)
				:EventBroadcastReciever(processor)
			{
			}
	
		public:
			BroadcastProcessor& GetProcessor()
			{
				return static_cast<BroadcastProcessor&>(EventBroadcastReciever::GetProcessor());
			}
			const BroadcastProcessor& GetProcessor() const
			{
				return static_cast<const BroadcastProcessor&>(EventBroadcastReciever::GetProcessor());
			}
	};
}

namespace Kortex
{
	class BroadcastEvent: public kxf::BasicEvent
	{
		public:
			BroadcastEvent() noexcept = default;

		public:
			std::unique_ptr<kxf::IEvent> Move() noexcept override
			{
				return std::make_unique<BroadcastEvent>(std::move(*this));
			}
	};
}
