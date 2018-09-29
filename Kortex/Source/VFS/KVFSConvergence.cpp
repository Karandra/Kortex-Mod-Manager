#include "stdafx.h"
#include "KVFSConvergence.h"
#include "KVFSService.h"

KVFSConvergence::KVFSConvergence(KVFSService* service, const wxString& mountPoint, const wxString& writeTarget)
	:KxVFSConvergence(service->GetServiceImpl(), mountPoint, writeTarget)
{
}
KVFSConvergence::~KVFSConvergence()
{
}

NTSTATUS KVFSConvergence::OnMount(EvtMounted& eventInfo)
{
	NTSTATUS statusCode = KxVFSConvergence::OnMount(eventInfo);

	KxBroadcastEvent(KEVT_VFS_MOUNTED).QueueClone();
	return statusCode;
}
NTSTATUS KVFSConvergence::OnUnMount(EvtUnMounted& eventInfo)
{
	NTSTATUS statusCode = KxVFSConvergence::OnUnMount(eventInfo);

	KxBroadcastEvent(KEVT_VFS_UNMOUNTED).QueueClone();
	return statusCode;
}
