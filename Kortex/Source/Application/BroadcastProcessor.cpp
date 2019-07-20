#include "stdafx.h"
#include "BroadcastProcessor.h"
#include "IApplication.h"

namespace Kortex
{
	BroadcastProcessor& BroadcastProcessor::Get()
	{
		return IApplication::GetInstance()->GetBroadcastProcessor();
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
