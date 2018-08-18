#include "stdafx.h"
#include "KBroadcastEvent.h"
#include "KApp.h"

KBroadcastEvent::KBroadcastEvent(wxEventType type)
	:wxNotifyEvent(type, 0)
{

}
KBroadcastEvent::~KBroadcastEvent()
{
}
KBroadcastEvent* KBroadcastEvent::Clone() const
{
	return new KBroadcastEvent(*this);
}
