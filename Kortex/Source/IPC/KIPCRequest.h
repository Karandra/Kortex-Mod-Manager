#pragma once
#include "stdafx.h"
#include "IPC/KIPC.h"
#include "VFS/KVirtualFileSystemBase.h"

template<size_t t_Length> class KIPCBasicStackString
{
	private:
		wchar_t m_Buffer[t_Length] = {0};

	public:
		KIPCBasicStackString(const wchar_t* s = NULL)
		{
			if (s)
			{
				wcscpy_s(m_Buffer, s);
			}
		}

	public:
		constexpr size_t GetLength() const
		{
			return t_Length;
		}
		const wchar_t* GetBuffer() const
		{
			return m_Buffer;
		}
		wchar_t* GetBuffer()
		{
			return m_Buffer;
		}

		bool operator==(const wxString& other) const
		{
			return m_Buffer == other;
		}
		bool operator!=(const wxString& other) const
		{
			return !(*this == other);
		}
		operator wxString() const
		{
			return wxString(GetBuffer());
		}
};
typedef KIPCBasicStackString<INT16_MAX> KIPCStackString;

//////////////////////////////////////////////////////////////////////////
class KIPCRequest
{
	public:
		static const wxChar* GetClassName()
		{
			return wxT("KIPCRequest");
		}
};

//////////////////////////////////////////////////////////////////////////
class KIPCRequest_InitVFSService: public KIPCRequest
{
	public:
		static const wxChar* GetClassName()
		{
			return wxT("KIPCRequest_InitVFSService");
		}
};
class KIPCRequest_UninstallVFSService: public KIPCRequest
{
	public:
		static const wxChar* GetClassName()
		{
			return wxT("KIPCRequest_UninstallVFSService");
		}
};

//////////////////////////////////////////////////////////////////////////
class KIPCRequest_EnableVFS: public KIPCRequest
{
	public:
		static const wxChar* GetClassName()
		{
			return wxT("KIPCRequest_EnableVFS");
		}

	private:
		bool m_ShouldEnable;

	public:
		KIPCRequest_EnableVFS(bool bShouldEnable)
			:m_ShouldEnable(bShouldEnable)
		{
		}

	public:
		bool ShouldEnable() const
		{
			return m_ShouldEnable;
		}
};
class KIPCRequest_VFSStateChanged: public KIPCRequest
{
	public:
		static const wxChar* GetClassName()
		{
			return wxT("KIPCRequest_VFSStateChanged");
		}

	private:
		bool m_IsEnabled;
		int m_Status = KVFS_STATUS_SUCCESS;

	public:
		KIPCRequest_VFSStateChanged(bool isEnabled, int status = KVFS_STATUS_SUCCESS)
			:m_IsEnabled(isEnabled), m_Status(status)
		{
		}

	public:
		bool IsEnabled() const
		{
			return m_IsEnabled;
		}
		int GetStatus() const
		{
			return m_Status;
		}
};

//////////////////////////////////////////////////////////////////////////
class KIPCRequest_CreateMirrorVFS: public KIPCRequest
{
	public:
		static const wxChar* GetClassName()
		{
			return wxT("KIPCRequest_CreateMirrorVFS");
		}

	private:
		KIPCStackString m_Source;
		KIPCStackString m_Target;

	public:
		KIPCRequest_CreateMirrorVFS(const wxString& source, const wxString& target);

	public:
		wxString GetSource() const
		{
			return m_Source;
		}
		wxString GetTarget() const
		{
			return m_Target;
		}
};
class KIPCRequest_ClearMirrorVFSList: public KIPCRequest
{
	public:
		static const wxChar* GetClassName()
		{
			return wxT("KIPCRequest_ClearMirrorVFSList");
		}
};

//////////////////////////////////////////////////////////////////////////
class KIPCRequest_CreateConvergenceVFS: public KIPCRequest
{
	public:
		static const wxChar* GetClassName()
		{
			return wxT("KIPCRequest_CreateConvergenceVFS");
		}

	private:
		KIPCStackString m_MountPoint;
		KIPCStackString m_WriteTarget;
		bool m_CanDeleteInVirtualFolder = false;

	public:
		KIPCRequest_CreateConvergenceVFS(const wxString& mountPoint, const wxString& writeTarget, bool canDeleteInVirtualFolder);

	public:
		wxString GetMountPoint() const
		{
			return m_MountPoint;
		}
		wxString GetWriteTarget() const
		{
			return m_WriteTarget;
		}
		bool CanDeleteInVirtualFolder() const
		{
			return m_CanDeleteInVirtualFolder;
		}
};

class KIPCRequest_AddConvergenceVirtualFolder: public KIPCRequest
{
	public:
		static const wxChar* GetClassName()
		{
			return wxT("KIPCRequest_AddConvergenceVirtualFolder");
		}

	private:
		KIPCStackString m_Path;

	public:
		KIPCRequest_AddConvergenceVirtualFolder(const wxString& path)
			:m_Path(path)
		{
		}

	public:
		wxString GetPath() const
		{
			return m_Path;
		}
};
class KIPCRequest_ClearConvergenceVirtualFolders: public KIPCRequest
{
	public:
		static const wxChar* GetClassName()
		{
			return wxT("KIPCRequest_ClearConvergenceVirtualFolders");
		}
};
