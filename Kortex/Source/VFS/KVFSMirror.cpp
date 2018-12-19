#include "stdafx.h"
#include "KVFSMirror.h"
#include "KVFSService.h"

KVFSMirror::KVFSMirror(KVFSService* service, const wxString& mountPoint, const wxString& source)
	:KxVFSMirror(service->GetServiceImpl(), mountPoint.wc_str(), source.wc_str())
{
}
KVFSMirror::~KVFSMirror()
{
}

NTSTATUS KVFSMirror::OnMount(EvtMounted& eventInfo)
{
	NTSTATUS statusCode = KxVFSMirror::OnMount(eventInfo);

	KxBroadcastEvent* event = new KxBroadcastEvent(KEVT_VFS_MOUNTED);
	event->Queue();

	return statusCode;
}
NTSTATUS KVFSMirror::OnUnMount(EvtUnMounted& eventInfo)
{
	NTSTATUS statusCode = KxVFSMirror::OnUnMount(eventInfo);

	KxBroadcastEvent* event = new KxBroadcastEvent(KEVT_VFS_UNMOUNTED);
	event->Queue();

	return statusCode;
}
