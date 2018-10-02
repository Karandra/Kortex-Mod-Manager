#include "stdafx.h"
#include "KVFSMultiMirror.h"
#include "KVFSService.h"

KVFSMultiMirror::KVFSMultiMirror(KVFSService* service, const wxString& mountPoint, const wxString& source)
	:KxVFSMultiMirror(service->GetServiceImpl(), mountPoint, source)
{
}
KVFSMultiMirror::~KVFSMultiMirror()
{
}

NTSTATUS KVFSMultiMirror::OnMount(EvtMounted& eventInfo)
{
	NTSTATUS statusCode = KxVFSConvergence::OnMount(eventInfo);

	KxBroadcastEvent* event = new KxBroadcastEvent(KEVT_VFS_MOUNTED);
	event->Queue();

	return statusCode;
}
NTSTATUS KVFSMultiMirror::OnUnMount(EvtUnMounted& eventInfo)
{
	NTSTATUS statusCode = KxVFSConvergence::OnUnMount(eventInfo);

	KxBroadcastEvent* event = new KxBroadcastEvent(KEVT_VFS_UNMOUNTED);
	event->Queue();

	return statusCode;
}
