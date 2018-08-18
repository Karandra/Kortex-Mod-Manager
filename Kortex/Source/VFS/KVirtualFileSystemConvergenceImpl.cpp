#include "stdafx.h"
#include "KVirtualFileSystemConvergenceImpl.h"
#include "KVirtualFileSystemBase.h"
#include "KVirtualFileSystemService.h"

KVirtualFileSystemConvergenceImpl::KVirtualFileSystemConvergenceImpl(KVirtualFileSystemService* service, KVirtualFileSystemBase* pKVFS, const wxString& mountPoint, const wxString& writeTarget)
	:KxVFSConvergence(service->GetServiceImpl(), mountPoint, writeTarget, 0, 100000), m_KVFS(pKVFS)
{
}
KVirtualFileSystemConvergenceImpl::~KVirtualFileSystemConvergenceImpl()
{
}

NTSTATUS KVirtualFileSystemConvergenceImpl::OnMount(DOKAN_MOUNTED_INFO* eventInfo)
{
	NTSTATUS statusCode = KxVFSConvergence::OnMount(eventInfo);

	wxNotifyEvent* event = new wxNotifyEvent(KVFSEVT_MOUNTED);
	event->SetEventObject(m_KVFS);
	event->SetString(GetMountPoint());
	m_KVFS->QueueEvent(event);
	return statusCode;
}
NTSTATUS KVirtualFileSystemConvergenceImpl::OnUnMount(DOKAN_UNMOUNTED_INFO* eventInfo)
{
	NTSTATUS statusCode = KxVFSConvergence::OnUnMount(eventInfo);

	wxNotifyEvent* event = new wxNotifyEvent(KVFSEVT_UNMOUNTED);
	event->SetEventObject(m_KVFS);
	event->SetString(GetMountPoint());
	m_KVFS->QueueEvent(event);
	return statusCode;
}
