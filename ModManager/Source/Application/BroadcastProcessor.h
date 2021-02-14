#pragma once
#include "Framework.hpp"
#include <kxf/EventSystem/EventBroadcastProcessor.h>

namespace Kortex
{
	class BroadcastReciever;
}

namespace Kortex
{
	class KORTEX_API BroadcastProcessor: public kxf::EventBroadcastProcessor
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
	class KORTEX_API BroadcastReciever: public kxf::EventBroadcastReciever
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
	class KORTEX_API BroadcastEvent: public kxf::BasicEvent
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
