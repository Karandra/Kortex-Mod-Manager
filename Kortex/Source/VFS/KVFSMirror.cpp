#include "stdafx.h"
#include "KVFSMirror.h"
#include "KVFSService.h"

KVFSMirror::KVFSMirror(KVFSService* service, const wxString& mountPoint, const wxString& source)
	:KxVFSMirror(service->GetServiceImpl(), mountPoint, source)
{
}
KVFSMirror::~KVFSMirror()
{
}

NTSTATUS KVFSMirror::OnMount(EvtMounted& eventInfo)
{
	NTSTATUS statusCode = KxVFSMirror::OnMount(eventInfo);

	KxBroadcastEvent(KEVT_VFS_MOUNTED).QueueClone();
	return statusCode;
}
NTSTATUS KVFSMirror::OnUnMount(EvtUnMounted& eventInfo)
{
	NTSTATUS statusCode = KxVFSMirror::OnUnMount(eventInfo);

	KxBroadcastEvent(KEVT_VFS_UNMOUNTED).QueueClone();
	return statusCode;
}
