#pragma once
#include "stdafx.h"
#include "IPC/KIPC.h"
#include "IPC/KIPCRequest.h"
#include "VFS/KVFSMirror.h"
#include <KxFramework/KxSingleton.h>
class KIPCConnection;
class KVFSService;
class KVFSConvergence;

class KIPCServer: public wxServer, public KxSingleton<KIPCServer>
{
	friend class KIPCConnection;

	public:
		static wxString GetClientFileName();

	private:
		const bool m_IsCreated = false;
		KIPCConnection* m_Connection = NULL;

		bool m_ManualDisablingInProgress = false;
		std::unique_ptr<KVFSService> m_Service;
		std::unique_ptr<KVFSConvergence> m_Convergence;
		std::vector<std::unique_ptr<KxVFSMirror>> m_MirrorVFSList;

		std::vector<std::pair<wxString, wxString>> m_ConvergenceIndex;

	private:
		virtual wxConnectionBase* OnAcceptConnection(const wxString& topic) override;
		virtual void OnDisconnect();

		void OnAcceptRequest(const KIPCRequestNS::InitVFSService& config);
		void OnAcceptRequest(const KIPCRequestNS::UninstallVFSService& config);

		void OnAcceptRequest(const KIPCRequestNS::CreateConvergenceVFS& config);
		void OnAcceptRequest(const KIPCRequestNS::ClearConvergenceVirtualFolders& config);
		void OnAcceptRequest(const KIPCRequestNS::AddConvergenceVirtualFolder& config);
		void OnAcceptRequest(const KIPCRequestNS::BuildConvergenceIndex& config);

		void OnAcceptRequest(const KIPCRequestNS::BeginConvergenceIndex& config);
		void OnAcceptRequest(const KIPCRequestNS::CommitConvergenceIndex& config);
		void OnAcceptRequest(const KIPCRequestNS::AddConvergenceIndex& config);

		void OnAcceptRequest(const KIPCRequestNS::CreateMirrorVFS& config);
		void OnAcceptRequest(const KIPCRequestNS::CreateMultiMirrorVFS& config);
		void OnAcceptRequest(const KIPCRequestNS::ClearMirrorVFSList& config);

		void OnAcceptRequest(const KIPCRequestNS::ToggleVFS& config);

		void OnVFSUnmounted(KxBroadcastEvent& event);
		void ReportMountError(int code);

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

		KVFSService* GetServiceVFS() const
		{
			return m_Service.get();
		}
		auto& GetMirrorVFSList()
		{
			return m_MirrorVFSList;
		}
		KVFSConvergence* GetConvergenceVFS() const
		{
			return m_Convergence.get();
		}

		bool IsVFSEnabled() const;
		int ToggleVFS();
		bool DisableVFS();
};
