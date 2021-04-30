#pragma once
#include "Framework.hpp"
#include <kxf/EventSystem/EventBroadcastProcessor.h>

namespace Kortex
{
	class BroadcastReceiver;
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
			bool AddReceiver(BroadcastReceiver& reciever);
			bool RemoveReceiver(BroadcastReceiver& reciever);
	};
}

namespace Kortex
{
	class KORTEX_API BroadcastReceiver: public kxf::EventBroadcastReceiver
	{
		public:
			BroadcastReceiver()
				:BroadcastReceiver(BroadcastProcessor::Get())
			{
			}
			BroadcastReceiver(BroadcastProcessor& processor)
				:EventBroadcastReceiver(processor)
			{
			}
	
		public:
			BroadcastProcessor& GetProcessor()
			{
				return static_cast<BroadcastProcessor&>(EventBroadcastReceiver::GetProcessor());
			}
			const BroadcastProcessor& GetProcessor() const
			{
				return static_cast<const BroadcastProcessor&>(EventBroadcastReceiver::GetProcessor());
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
