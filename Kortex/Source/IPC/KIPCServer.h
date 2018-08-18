#pragma once
#include "stdafx.h"
#include "IPC/KIPC.h"
#include "VFS/KVirtualFileSystemMirror.h"
class KIPCConnection;
class KIPCRequest_InitVFSService;
class KIPCRequest_UninstallVFSService;
class KIPCRequest_CreateMirrorVFS;
class KIPCRequest_ClearMirrorVFSList;
class KIPCRequest_CreateConvergenceVFS;
class KIPCRequest_AddConvergenceVirtualFolder;
class KIPCRequest_ClearConvergenceVirtualFolders;
class KIPCRequest_EnableVFS;
class KVirtualFileSystemService;
class KVirtualFileSystemConvergence;

class KIPCServer: public wxServer
{
	friend class KIPCConnection;

	public:
	#if KIPC_SERVER
		static KIPCServer& Get();
	#endif
		static wxString GetClientFileName();

	private:
		const bool m_IsCreated = false;
		KIPCConnection* m_Connection = NULL;

		bool m_ManualDisablingInProgress = false;
		std::unique_ptr<KVirtualFileSystemService> m_Service;
		std::unique_ptr<KVirtualFileSystemConvergence> m_Convergence;
		std::vector<std::unique_ptr<KVirtualFileSystemMirror>> m_MirrorVFSList;

	private:
		virtual wxConnectionBase* OnAcceptConnection(const wxString& topic) override;
		virtual void OnDisconnect();

		void OnInitService(const KIPCRequest_InitVFSService& config);
		void OnUninstallService(const KIPCRequest_UninstallVFSService& config);
		void OnCreateConvergenceVFS(const KIPCRequest_CreateConvergenceVFS& config);
		void OnClearConvergenceVirtualFolders(const KIPCRequest_ClearConvergenceVirtualFolders& config);
		void OnAddConvergenceVirtualFolder(const KIPCRequest_AddConvergenceVirtualFolder& config);
		void OnCreateMirrorVFS(const KIPCRequest_CreateMirrorVFS& config);
		void OnClearMirrorVFSList(const KIPCRequest_ClearMirrorVFSList& config);
		void OnEnableVFS(const KIPCRequest_EnableVFS& config);

		void OnVFSUnmounted(wxNotifyEvent& event);
		void ReportMountError(int code, KVirtualFileSystemBase* vfs);

	public:
		KIPCServer();
		virtual ~KIPCServer();

	public:
		bool IsCreated() const
		{
			return m_IsCreated;
		}
		bool IsConnected() const
		{
			return m_Connection != NULL;
		}
		KIPCConnection* GetConnection() const
		{
			return m_Connection;
		}

		KVirtualFileSystemService* GetServiceVFS() const
		{
			return m_Service.get();
		}
		auto& GetMirrorVFSList()
		{
			return m_MirrorVFSList;
		}
		KVirtualFileSystemConvergence* GetConvergenceVFS() const
		{
			return m_Convergence.get();
		}

		bool IsVFSEnabled() const;
		int EnableVFS();
		bool DisableVFS();
};
