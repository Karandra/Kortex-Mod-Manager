#pragma once
#include "stdafx.h"
#include "IPC/KIPC.h"
class KIPCConnection;
class KIPCRequest_VFSStateChanged;
class KIPCRequest_AddDownload;

class KIPCClient: public wxClient
{
	friend class KIPCConnection;

	public:
		static KIPCClient& Get();
		static wxString GetServerFileName();
		static bool RunServerAndConnect(KIPCClient** clientInstance);

	private:
		KIPCConnection* m_Connection = NULL;

	private:
		bool CreateConnection();
		virtual wxConnectionBase* OnMakeConnection() override;
		virtual void OnDisconnect();

		void OnVFSStateChanged(const KIPCRequest_VFSStateChanged& config);

	public:
		KIPCClient();
		virtual ~KIPCClient();

	public:
		bool IsConnected() const
		{
			return m_Connection != NULL;
		}

		bool InitConnection();
		bool Disconnect();
		bool InitVFSService();
		bool UninstallVFSService();

		bool CreateVFS_Mirror(const wxString& source, const wxString& target);
		bool MirrorVFS_ClearList();

		bool CreateVFS_Convergence(const wxString& source, const wxString& writeTarget, const KxStringVector& virtualFolders, bool canDeleteInVirtualFolder);
		bool ConvergenceVFS_ClearVirtualFolders();
		
		bool EnableVFS();
		bool DisableVFS();
};
