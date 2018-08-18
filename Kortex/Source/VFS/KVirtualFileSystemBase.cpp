#include "stdafx.h"
#include "KVirtualFileSystemBase.h"
#include "KVirtualFileSystemService.h"
#include "KApp.h"
#include <KxFramework/KxFile.h>
#include "KxVFSBase.h"

wxDEFINE_EVENT(KVFSEVT_MOUNTED, wxNotifyEvent);
wxDEFINE_EVENT(KVFSEVT_UNMOUNTED, wxNotifyEvent);

bool KVirtualFileSystemBase::UnMountDirectory(const wxString& mountPoint)
{
	return KxVFSBase::UnMountDirectory(mountPoint);
}
bool KVirtualFileSystemBase::IsSuccessCode(int code)
{
	return KxVFSBase::IsCodeSuccess(code);
}
int KVirtualFileSystemBase::GetSuccessCode()
{
	return KVFS_STATUS_SUCCESS;
}
wxString KVirtualFileSystemBase::GetStatusCodeMessage(int code)
{
	if (code != KVFS_STATUS_NOT_STARTED && (code < KVFS_STATUS_MIN || code > KVFS_STATUS_MAX))
	{
		code = KVFS_STATUS_UNKNOWN_CODE;
	}
	#if KIPC_SERVER
	return wxString::Format("%d", (int)std::abs(code));
	#else
	return T(wxString::Format("VFS.Error%d", (int)std::abs(code)));
	#endif
}

KVirtualFileSystemBase::KVirtualFileSystemBase(KVirtualFileSystemService* service)
	:m_KVFS(service)
{
}
KVirtualFileSystemBase::~KVirtualFileSystemBase()
{
}

bool KVirtualFileSystemBase::IsOK()
{
	return GetImpl() != NULL;
}

int KVirtualFileSystemBase::Mount()
{
	// Remove existing folder if empty and create a new one.
	// Just in case. Dokan sometimes can't mount VFS into existing folder event if it's empty
	KxFile mountPoint(GetMountPoint());
	mountPoint.RemoveFolder(true);
	mountPoint.CreateFolder();

	return GetImpl()->Mount();
}
bool KVirtualFileSystemBase::UnMount()
{
	return GetImpl()->UnMount();
}
bool KVirtualFileSystemBase::IsMounted() const
{
	return GetImpl()->IsMounted();
}

wxString KVirtualFileSystemBase::GetVolumeName() const
{
	return GetImpl()->GetVolumeName();
}
wxString KVirtualFileSystemBase::GetVolumeFileSystemName() const
{
	return GetImpl()->GetVolumeFileSystemName();
}
ULONG KVirtualFileSystemBase::GetVolumeSerialNumber() const
{
	return GetImpl()->GetVolumeSerialNumber();
}

KVirtualFileSystemService* KVirtualFileSystemBase::GetService() const
{
	return m_KVFS;
}
wxString KVirtualFileSystemBase::GetMountPoint() const
{
	return GetImpl()->GetMountPoint();
}
bool KVirtualFileSystemBase::SetMountPoint(const wxString& mountPoint)
{
	return GetImpl()->SetMountPoint(mountPoint);
}
ULONG KVirtualFileSystemBase::GetFlags() const
{
	return GetImpl()->GetFlags();
}
bool KVirtualFileSystemBase::SetFlags(ULONG flags)
{
	return GetImpl()->SetFlags(flags);
}

NTSTATUS KVirtualFileSystemBase::GetNtStatusByWin32ErrorCode(DWORD win32ErrorCode) const
{
	return GetImpl()->GetNtStatusByWin32ErrorCode(win32ErrorCode);
}
NTSTATUS KVirtualFileSystemBase::GetNtStatusByWin32LastErrorCode() const
{
	return GetImpl()->GetNtStatusByWin32LastErrorCode();
}
