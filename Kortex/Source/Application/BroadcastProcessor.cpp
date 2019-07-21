#include "stdafx.h"
#include "BroadcastProcessor.h"
#include "SystemApplication.h"

namespace Kortex
{
	BroadcastProcessor& BroadcastProcessor::Get()
	{
		return SystemApplication::GetInstance()->GetBroadcastProcessor();
	}

	bool BroadcastProcessor::AddReciever(BroadcastReciever& reciever)
	{
		return KxBroadcastProcessor::AddReciever(reciever);
	}
	bool BroadcastProcessor::RemoveReciever(BroadcastReciever& reciever)
	{
		return KxBroadcastProcessor::RemoveReciever(reciever);
	}
}
