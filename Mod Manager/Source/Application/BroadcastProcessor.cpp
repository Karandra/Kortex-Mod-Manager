#include "pch.hpp"
#include "BroadcastProcessor.h"
#include "SystemApplication.h"

namespace Kortex
{
	BroadcastProcessor& BroadcastProcessor::Get()
	{
		return SystemApplication::GetInstance().GetBroadcastProcessor();
	}

	bool BroadcastProcessor::AddReciever(BroadcastReciever& reciever)
	{
		return EventBroadcastProcessor::AddReciever(reciever);
	}
	bool BroadcastProcessor::RemoveReciever(BroadcastReciever& reciever)
	{
		return EventBroadcastProcessor::RemoveReciever(reciever);
	}
}
