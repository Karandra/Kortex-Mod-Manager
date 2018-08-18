#pragma once
#include "stdafx.h"
class KxVFSBase;
class KVirtualFileSystemService;

enum KVirtualFileSystemStatus
{
	KVFS_STATUS_SUCCESS = 0,
	KVFS_STATUS_ERROR = -1,
	KVFS_STATUS_DRIVE_LETTER_ERROR = -2,
	KVFS_STATUS_DRIVER_INSTALL_ERROR = -3,
	KVFS_STATUS_START_ERROR = -4,
	KVFS_STATUS_MOUNT_ERROR = -5,
	KVFS_STATUS_MOUNT_POINT_ERROR = -6,
	KVFS_STATUS_VERSION_ERROR = -7,

	KVFS_STATUS_MIN = KVFS_STATUS_VERSION_ERROR,
	KVFS_STATUS_MAX = KVFS_STATUS_SUCCESS,

	KVFS_STATUS_NOT_STARTED = 1000,
	KVFS_STATUS_UNKNOWN_CODE = 10000,
};

wxDECLARE_EVENT(KVFSEVT_MOUNTED, wxNotifyEvent);
wxDECLARE_EVENT(KVFSEVT_UNMOUNTED, wxNotifyEvent);

class KVirtualFileSystemBase: public wxEvtHandler
{
	public:
		static bool UnMountDirectory(const wxString& mountPoint);
		static bool IsSuccessCode(int code);
		static int GetSuccessCode();
		static wxString GetStatusCodeMessage(int code);

	private:
		KVirtualFileSystemService* m_KVFS = NULL;

	protected:
		virtual KxVFSBase* GetImpl() const = 0;

	public:
		KVirtualFileSystemBase(KVirtualFileSystemService* service);
		virtual ~KVirtualFileSystemBase();

	public:
		bool IsOK();

		int Mount();
		bool UnMount();
		bool IsMounted() const;

		wxString GetVolumeName() const;
		wxString GetVolumeFileSystemName() const;
		ULONG GetVolumeSerialNumber() const;

		KVirtualFileSystemService* GetService() const;
		wxString GetMountPoint() const;
		bool SetMountPoint(const wxString& mountPoint);
		ULONG GetFlags() const;
		bool SetFlags(ULONG flags);

		NTSTATUS GetNtStatusByWin32ErrorCode(DWORD win32ErrorCode) const;
		NTSTATUS GetNtStatusByWin32LastErrorCode() const;
};
typedef std::vector<KVirtualFileSystemBase*> KVirtualFileSystemArray;
