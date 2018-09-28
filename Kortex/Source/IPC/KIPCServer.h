#pragma once
#include "stdafx.h"
#include "IPC/KIPC.h"
#include "IPC/KIPCRequest.h"
#include "VFS/KVirtualFileSystemMirror.h"
#include <KxFramework/KxSingleton.h>
class KIPCConnection;
class KVirtualFileSystemService;
class KVirtualFileSystemConvergence;

class KIPCServer: public wxServer, public KxSingleton<KIPCServer>
{
	friend class KIPCConnection;

	public:
		static wxString GetClientFileName();

	private:
		const bool m_IsCreated = false;
		KIPCConnection* m_Connection = NULL;

		bool m_ManualDisablingInProgress = false;
		std::unique_ptr<KVirtualFileSystemService> m_Service;
		std::unique_ptr<KVirtualFileSystemConvergence> m_Convergence;
		std::vector<std::unique_ptr<KVirtualFileSystemMirror>> m_MirrorVFSList;

		std::vector<std::pair<wxString, wxString>> m_ConvergenceIndex;

	private:
		virtual wxConnectionBase* OnAcceptConnection(const wxString& topic) override;
		virtual void OnDisconnect();

		void OnInitService(const KIPCRequestNS::InitVFSService& config);
		void OnUninstallService(const KIPCRequestNS::UninstallVFSService& config);

		void OnCreateConvergenceVFS(const KIPCRequestNS::CreateConvergenceVFS& config);
		void OnClearConvergenceVirtualFolders(const KIPCRequestNS::ClearConvergenceVirtualFolders& config);
		void OnAddConvergenceVirtualFolder(const KIPCRequestNS::AddConvergenceVirtualFolder& config);
		void OnBuildConvergenceIndex(const KIPCRequestNS::BuildConvergenceIndex& config);

		void OnBeginConvergenceIndex(const KIPCRequestNS::BeginConvergenceIndex& config);
		void OnCommitConvergenceIndex(const KIPCRequestNS::CommitConvergenceIndex& config);
		void OnAddConvergenceIndex(const KIPCRequestNS::AddConvergenceIndex& config);

		void OnCreateMirrorVFS(const KIPCRequestNS::CreateMirrorVFS& config);
		void OnClearMirrorVFSList(const KIPCRequestNS::ClearMirrorVFSList& config);

		void OnEnableVFS(const KIPCRequestNS::EnableVFS& config);

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
