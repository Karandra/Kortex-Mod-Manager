#pragma once
#include "stdafx.h"
#include "IPC/KIPC.h"
#include "VFS/KVFSService.h"
#include "VFS/KVFSMirror.h"
#include "VFS/KVFSConvergence.h"
#include <KxFramework/KxSharedMemory.h>
#include <array>
class KIPCConnection;
class KIPCClient;
class KIPCServer;

namespace KIPCRequestNS
{
	template<size_t t_Length> class BasicStaticString
	{
		private:
			wchar_t m_Buffer[t_Length] = {0};
			size_t m_Length = 0;

		public:
			BasicStaticString(const wchar_t* s = NULL)
			{
				if (s)
				{
					m_Length = wcslen(s);
					wcsncpy_s(m_Buffer, s, m_Length);
				}
			}
			BasicStaticString(const wxString& s)
				:BasicStaticString(s.wc_str())
			{
			}

		public:
			constexpr size_t GetLength() const noexcept
			{
				return t_Length;
			}
			const wchar_t* GetBuffer() const noexcept
			{
				return m_Buffer;
			}
			wchar_t* GetBuffer() noexcept
			{
				return m_Buffer;
			}

			const wchar_t* data() const noexcept
			{
				return m_Buffer;
			}
			wchar_t* data() noexcept
			{
				return m_Buffer;
			}
			size_t size() const noexcept
			{
				return m_Length;
			}
			size_t length() const noexcept
			{
				return m_Length;
			}
			bool empry() const
			{
				return m_Length == 0;
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
				return wxString(m_Buffer, m_Length);
			}
	};
	using StaticString = BasicStaticString<INT16_MAX>; // INT16_MAX is maximum file path length in Windows

	//////////////////////////////////////////////////////////////////////////
	extern const wxChar* TypeName[];
	enum class TypeID
	{
		None = 0,

		InitVFSService,
		UninstallVFSService,

		ToggleVFS,
		VFSStateChanged,

		CreateMirrorVFS,
		CreateMultiMirrorVFS,
		ClearMirrorVFSList,

		CreateConvergenceVFS,
		AddConvergenceVirtualFolder,
		ClearConvergenceVirtualFolders,
		BuildConvergenceIndex,

		BeginConvergenceIndex,
		CommitConvergenceIndex,
		AddConvergenceIndex,

		MAX,
	};

	class BaseRequest
	{
		friend class KIPCConnection;

		public:
			using TypeID = KIPCRequestNS::TypeID;
			template<class T> using MemRegionRO = KxSharedMemory<T, KxSharedMemoryNS::Protection::Read>;
			template<class T> using MemRegionRW = KxSharedMemory<T, KxSharedMemoryNS::Protection::RW>;

		private:
			TypeID m_TypeID = TypeID::None;

		protected:
			wxString GetSharedDataName() const
			{
				return wxString(wxS("Kortex/IPC/")) + GetTypeName();
			}
			template<class T> static MemRegionRO<T> GetSharedData(const wxString& name)
			{
				MemRegionRO<T> region;
				region.Open(name);
				return region;
			}

		public:
			BaseRequest(TypeID typeID)
				:m_TypeID(typeID)
			{
			}

		public:
			TypeID GetTypeID() const
			{
				return m_TypeID;
			}
			const wxChar* GetTypeName() const
			{
				return TypeName[(size_t)m_TypeID];
			}
			bool IsSameType(const BaseRequest& other) const
			{
				return m_TypeID == other.m_TypeID;
			}
			
			template<class T, class... Args> MemRegionRW<T> CreateSharedData(Args&&... args) const
			{
				return MemRegionRW<T>(GetSharedDataName(), std::forward<Args>(args)...);
			}
			template<class T> MemRegionRO<T> GetSharedData() const
			{
				return GetSharedData<T>(GetSharedDataName());
			}
	};

	template<TypeID t_TypeID> class BaseRequestType: public BaseRequest
	{
		public:
			static constexpr TypeID GetClassTypeID()
			{
				return t_TypeID;
			}
			static constexpr const wxChar* GetClassTypeName()
			{
				return TypeName[(size_t)t_TypeID];
			}

		public:
			BaseRequestType()
				:BaseRequest(t_TypeID)
			{
			}
	};

	//////////////////////////////////////////////////////////////////////////
	using InitVFSService = BaseRequestType<TypeID::InitVFSService>;
	using UninstallVFSService = BaseRequestType<TypeID::UninstallVFSService>;
	
	//////////////////////////////////////////////////////////////////////////
	class ToggleVFS: public BaseRequestType<TypeID::ToggleVFS>
	{
		private:
			bool m_ShouldEnable = false;

		public:
			ToggleVFS(bool shouldEnable)
				:m_ShouldEnable(shouldEnable)
			{
			}

		public:
			bool ShouldEnable() const
			{
				return m_ShouldEnable;
			}
	};
	class VFSStateChanged: public BaseRequestType<TypeID::VFSStateChanged>
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
	class CreateMirrorVFS: public BaseRequestType<TypeID::CreateMirrorVFS>
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
	class CreateMultiMirrorVFS: public BaseRequestType<TypeID::CreateMultiMirrorVFS>
	{
		private:
			std::array<StaticString, 8> m_Sources;
			StaticString m_Target;

		public:
			CreateMultiMirrorVFS(const KxStringVector& sources, const wxString& target)
				:m_Target(target)
			{
				m_Sources.fill(StaticString());
				for (size_t i = 0; i < std::min(sources.size(), m_Sources.size()); i++)
				{
					m_Sources[i] = sources[i];
				}
			}

		public:
			KxStringVector GetSources() const
			{
				KxStringVector sources;
				for (const StaticString& s: m_Sources)
				{
					if (!s.empry())
					{
						sources.push_back(s);
					}
				}
				return sources;
			}
			wxString GetTarget() const
			{
				return m_Target;
			}
	};
	using ClearMirrorVFSList = BaseRequestType<TypeID::ClearMirrorVFSList>;

	//////////////////////////////////////////////////////////////////////////
	class CreateConvergenceVFS: public BaseRequestType<TypeID::CreateConvergenceVFS>
	{
		private:
			StaticString m_MountPoint;
			StaticString m_WriteTarget;
			bool m_CanDeleteInVirtualFolder = false;

		public:
			CreateConvergenceVFS(const wxString& mountPoint, const wxString& writeTarget, bool canDeleteInVirtualFolder)
				:m_MountPoint(mountPoint), m_WriteTarget(writeTarget), m_CanDeleteInVirtualFolder(canDeleteInVirtualFolder)
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
	class AddConvergenceVirtualFolder: public BaseRequestType<TypeID::AddConvergenceVirtualFolder>
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
	using BuildConvergenceIndex = BaseRequestType<TypeID::BuildConvergenceIndex>;
	using ClearConvergenceVirtualFolders = BaseRequestType<TypeID::ClearConvergenceVirtualFolders>;

	//////////////////////////////////////////////////////////////////////////
	using BeginConvergenceIndex = BaseRequestType<TypeID::BeginConvergenceIndex>;
	using CommitConvergenceIndex = BaseRequestType<TypeID::CommitConvergenceIndex>;

	class AddConvergenceIndex: public BaseRequestType<TypeID::AddConvergenceIndex>
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
