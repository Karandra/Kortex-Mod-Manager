#include "stdafx.h"
#include "KVirtualFileSystemMirrorImpl.h"
#include "KVirtualFileSystemBase.h"
#include "KVirtualFileSystemService.h"

KVirtualFileSystemMirrorImpl::KVirtualFileSystemMirrorImpl(KVirtualFileSystemService* service, KVirtualFileSystemBase* pKVFS, const wxString& mountPoint, const wxString& source)
	:KxVFSMirror(service->GetServiceImpl(), mountPoint, source), m_KVFS(pKVFS)
{
}
KVirtualFileSystemMirrorImpl::~KVirtualFileSystemMirrorImpl()
{
}

NTSTATUS KVirtualFileSystemMirrorImpl::OnMount(DOKAN_MOUNTED_INFO* eventInfo)
{
	NTSTATUS statusCode = KxVFSMirror::OnMount(eventInfo);

	wxNotifyEvent* event = new wxNotifyEvent(KVFSEVT_MOUNTED);
	event->SetEventObject(m_KVFS);
	event->SetString(GetMountPoint());
	m_KVFS->QueueEvent(event);
	return statusCode;
}
NTSTATUS KVirtualFileSystemMirrorImpl::OnUnMount(DOKAN_UNMOUNTED_INFO* eventInfo)
{
	NTSTATUS statusCode = KxVFSMirror::OnUnMount(eventInfo);

	wxNotifyEvent* event = new wxNotifyEvent(KVFSEVT_UNMOUNTED);
	event->SetEventObject(m_KVFS);
	event->SetString(GetMountPoint());
	m_KVFS->QueueEvent(event);
	return statusCode;
}
