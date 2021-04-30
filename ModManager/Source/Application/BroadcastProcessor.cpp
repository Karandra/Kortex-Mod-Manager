#include "pch.hpp"
#include "BroadcastProcessor.h"
#include "SystemApplication.h"

namespace Kortex
{
	BroadcastProcessor& BroadcastProcessor::Get()
	{
		return SystemApplication::GetInstance().GetBroadcastProcessor();
	}

	bool BroadcastProcessor::AddReceiver(BroadcastReceiver& reciever)
	{
		return EventBroadcastProcessor::AddReceiver(reciever);
	}
	bool BroadcastProcessor::RemoveReceiver(BroadcastReceiver& reciever)
	{
		return EventBroadcastProcessor::RemoveReceiver(reciever);
	}
}
