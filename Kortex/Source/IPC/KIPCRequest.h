#pragma once
#include "stdafx.h"
#include "IPC/KIPC.h"
#include "VFS/KVirtualFileSystemBase.h"
#include "VFS/KVirtualFileSystemMirror.h"
#include "VFS/KVirtualFileSystemConvergence.h"
#include <KxFramework/KxSharedMemory.h>

namespace KIPCRequestNS
{
	template<size_t t_Length> class BasicStaticString
	{
		private:
			wchar_t m_Buffer[t_Length] = {0};

		public:
			BasicStaticString(const wchar_t* s = NULL)
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
				return wxString(m_Buffer);
			}
	};
	using StaticString = BasicStaticString<INT16_MAX>; // INT16_MAX is maximum file path length in Windows

	//////////////////////////////////////////////////////////////////////////
	enum class Type
	{
		None = 0,

		InitVFSService,
		UninstallVFSService,

		EnableVFS,
		VFSStateChanged,

		CreateMirrorVFS,
		ClearMirrorVFSList,

		CreateConvergenceVFS,
		AddConvergenceVirtualFolder,
		ClearConvergenceVirtualFolders,
		BuildConvergenceIndex,

		BeginConvergenceIndex,
		CommitConvergenceIndex,
		AddConvergenceIndex,

		COUNT,
	};
	extern const wxChar* TypeName[(size_t)Type::COUNT];

	class BaseRequest
	{
		private:
			template<class T> using MemRegionRO = KxSharedMemory<T, KxSharedMemoryNS::Protection::Read>;
			template<class T> using MemRegionRW = KxSharedMemory<T, KxSharedMemoryNS::Protection::RW>;

		private:
			Type m_Type = Type::None;

		public:
			BaseRequest(Type type)
				:m_Type(type)
			{
			}

		public:
			Type GetRequestType() const
			{
				return m_Type;
			}
			const wxChar* GetRequestName() const
			{
				return TypeName[(size_t)m_Type];
			}
			
			wxString GetSharedRegionName() const
			{
				return wxString(wxS("Kortex/IPC/")) + GetRequestName();
			}
			template<class T, class... Args> MemRegionRW<T> CreateSharedMemoryRegion(Args&&... args) const
			{
				return MemRegionRW<T>(GetSharedRegionName(), std::forward<Args>(args)...);
			}
			template<class T> MemRegionRO<T> GetSharedMemoryRegion() const
			{
				return MemRegionRO<T>(GetSharedRegionName());
			}

			bool IsSameType(const BaseRequest& other) const
			{
				return m_Type == other.GetRequestType();
			}
	};

	template<Type t_Type = Type::None> class BaseRequestType: public BaseRequest
	{
		public:
			static const wxChar* GetClassName()
			{
				return TypeName[(size_t)t_Type];
			}

		public:
			BaseRequestType()
				:BaseRequest(t_Type)
			{
			}
	};

	//////////////////////////////////////////////////////////////////////////
	class InitVFSService: public BaseRequestType<Type::InitVFSService>
	{
	};
	class UninstallVFSService: public BaseRequestType<Type::UninstallVFSService>
	{
	};

	//////////////////////////////////////////////////////////////////////////
	class EnableVFS: public BaseRequestType<Type::EnableVFS>
	{
		private:
			bool m_ShouldEnable = false;

		public:
			EnableVFS(bool bShouldEnable)
				:m_ShouldEnable(bShouldEnable)
			{
			}

		public:
			bool ShouldEnable() const
			{
				return m_ShouldEnable;
			}
	};
	class VFSStateChanged: public BaseRequestType<Type::VFSStateChanged>
	{
		private:
			bool m_IsEnabled = false;
			int m_Status = KVFS_STATUS_SUCCESS;

		public:
			VFSStateChanged(bool isEnabled, int status = KVFS_STATUS_SUCCESS)
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
	class CreateMirrorVFS: public BaseRequestType<Type::CreateMirrorVFS>
	{
		private:
			StaticString m_Source;
			StaticString m_Target;

		public:
			CreateMirrorVFS(const wxString& source, const wxString& target)
				:m_Source(source), m_Target(target)
			{
			}

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
	class ClearMirrorVFSList: public BaseRequestType<Type::ClearMirrorVFSList>
	{
	};

	//////////////////////////////////////////////////////////////////////////
	class CreateConvergenceVFS: public BaseRequestType<Type::CreateConvergenceVFS>
	{
		private:
			StaticString m_MountPoint;
			StaticString m_WriteTarget;
			bool m_CanDeleteInVirtualFolder = false;

		public:
			CreateConvergenceVFS(const wxString& mountPoint, const wxString& writeTarget, bool canDeleteInVirtualFolder)
				: m_MountPoint(mountPoint), m_WriteTarget(writeTarget), m_CanDeleteInVirtualFolder(canDeleteInVirtualFolder)
			{
			}

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
	class AddConvergenceVirtualFolder: public BaseRequestType<Type::AddConvergenceVirtualFolder>
	{
		private:
			StaticString m_Path;

		public:
			AddConvergenceVirtualFolder(const wxString& path)
				:m_Path(path)
			{
			}

		public:
			wxString GetPath() const
			{
				return m_Path;
			}
	};
	class ClearConvergenceVirtualFolders: public BaseRequestType<Type::ClearConvergenceVirtualFolders>
	{
	};
	class BuildConvergenceIndex: public BaseRequestType<Type::BuildConvergenceIndex>
	{
	};

	//////////////////////////////////////////////////////////////////////////
	class BeginConvergenceIndex: public BaseRequestType<Type::BeginConvergenceIndex>
	{
		private:
			size_t m_InitialSize = 0;

		public:
			BeginConvergenceIndex(size_t initialiIze = 0)
				:m_InitialSize(initialiIze)
			{
			}

		public:
			size_t GetInitialSize() const
			{
				return m_InitialSize;
			}
	};
	class CommitConvergenceIndex: public BaseRequestType<Type::CommitConvergenceIndex>
	{
	};
	class AddConvergenceIndex: public BaseRequestType<Type::AddConvergenceIndex>
	{
		private:
			StaticString m_RequestPath;
			StaticString m_TargetPath;

		public:
			AddConvergenceIndex(const wxString& requestPath, const wxString& targetPath)
				:m_RequestPath(requestPath), m_TargetPath(targetPath)
			{
			}

		public:
			wxString GetRequestPath() const
			{
				return m_RequestPath;
			}
			wxString GetTargetPath() const
			{
				return m_TargetPath;
			}
	};
};

using KIPCRequest = KIPCRequestNS::BaseRequest;
