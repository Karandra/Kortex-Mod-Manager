#include "stdafx.h"
#include "KVFSConvergence.h"
#include "KVFSService.h"

KVFSConvergence::KVFSConvergence(KVFSService* service, const wxString& mountPoint, const wxString& writeTarget)
	:KxVFSConvergence(service->GetServiceImpl(), mountPoint.wc_str(), writeTarget.wc_str())
{
}
KVFSConvergence::~KVFSConvergence()
{
}

NTSTATUS KVFSConvergence::OnMount(EvtMounted& eventInfo)
{
	NTSTATUS statusCode = KxVFSConvergence::OnMount(eventInfo);

	KxBroadcastEvent* event = new KxBroadcastEvent(KEVT_VFS_MOUNTED);
	event->Queue();

	return statusCode;
}
NTSTATUS KVFSConvergence::OnUnMount(EvtUnMounted& eventInfo)
{
	NTSTATUS statusCode = KxVFSConvergence::OnUnMount(eventInfo);

	KxBroadcastEvent* event = new KxBroadcastEvent(KEVT_VFS_UNMOUNTED);
	event->Queue();

	return statusCode;
}
