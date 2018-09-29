#pragma once
#include "stdafx.h"
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxBroadcastEvent.h>
#include "KxVFSBase.h"

enum KVFSStatus
{
	KVFS_STATUS_SUCCESS = DOKAN_SUCCESS,
	KVFS_STATUS_ERROR = DOKAN_ERROR,
	KVFS_STATUS_DRIVE_LETTER_ERROR = DOKAN_DRIVE_LETTER_ERROR,
	KVFS_STATUS_DRIVER_INSTALL_ERROR = DOKAN_DRIVER_INSTALL_ERROR,
	KVFS_STATUS_START_ERROR = DOKAN_START_ERROR,
	KVFS_STATUS_MOUNT_ERROR = DOKAN_MOUNT_ERROR,
	KVFS_STATUS_MOUNT_POINT_ERROR = DOKAN_MOUNT_POINT_ERROR,
	KVFS_STATUS_VERSION_ERROR = DOKAN_VERSION_ERROR,

	KVFS_STATUS_MIN = KVFS_STATUS_VERSION_ERROR,
	KVFS_STATUS_MAX = KVFS_STATUS_SUCCESS,

	KVFS_STATUS_NOT_STARTED = 1000,
	KVFS_STATUS_UNKNOWN_CODE = 10000,
};

class KxVFSService;
class KVFSService: public KxSingletonPtr<KVFSService>
{
	public:
		static wxString GetLibraryVersion();
		static wxString GetDokanVersion();

		static int GetSuccessCode();
		static bool IsSuccessCode(int code);
		static wxString GetStatusCodeMessage(int code);

	private:
		static wxString InitLibraryPath();
		static wxString InitDriverPath();

	private:
		const wxString m_Name;
		const wxString m_DisplayName;
		const wxString m_Description;
		const wxString m_LibraryPath;
		const wxString m_DriverPath;
		KxVFSService* m_ServiceImpl = NULL;
		HMODULE m_LibraryHandle = NULL;

	private:
		void UnInit();

	public:
		KVFSService();
		virtual ~KVFSService();
		bool Init();

	public:
		bool IsOK() const;

		const wxString& GetName() const
		{
			return m_Name;
		}
		const wxString& GetDisplayName() const
		{
			return m_DisplayName;
		}
		const wxString& GetDescription() const
		{
			return m_Description;
		}
		const wxString& GetLibraryPath() const
		{
			return m_LibraryPath;
		}
		const wxString& GetDriverPath() const
		{
			return m_DriverPath;
		}

		KxVFSService* GetServiceImpl() const;
		bool IsReady() const;
		bool IsStarted() const;
		bool IsInstalled() const;

		bool Start();
		bool Stop();
		bool Install();
		bool Uninstall();
};

wxDECLARE_EVENT(KEVT_VFS_MOUNTED, KxBroadcastEvent);
wxDECLARE_EVENT(KEVT_VFS_UNMOUNTED, KxBroadcastEvent);
