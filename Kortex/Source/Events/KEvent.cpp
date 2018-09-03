#include "stdafx.h"
#include "KEvent.h"

KEvent::KEvent(wxEventType type)
	:wxNotifyEvent(type)
{
	// Default behavior is to walk this event through entire handlers tree to the wxApp
	StopPropagation();
	Skip();
}
KEvent::~KEvent()
{
}
KEvent* KEvent::Clone() const
{
	return new KEvent(*this);
}
