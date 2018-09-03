#include "stdafx.h"
#include "KIPCClient.h"
#include "KIPCConnection.h"
#include "KIPCRequest.h"
#include "KApp.h"
#include "ModManager/KModManager.h"
#include "PluginManager/KPluginManager.h"
#include "PluginManager/KPluginManagerWorkspace.h"
#include "Events/KVFSEvent.h"
#include "Events/KLogEvent.h"
#include "VFS/KVirtualFileSystemBase.h"
#include <KxFramework/KxSystem.h>
#include <KxFramework/KxProcess.h>
#include <KxFramework/KxProgressDialog.h>

KxSingletonPtr_Define(KIPCClient);

wxString KIPCClient::GetServerFileName()
{
	wxString path = "Kortex Server";
	if (KxSystem::Is64Bit())
	{
		path += " x64";
	}
	path += ".exe";
	return path;
}
bool KIPCClient::RunServerAndConnect(KIPCClient** clientInstance)
{
	bool isRunSuccess = false;
	KxProcess serverExec(GetServerFileName());
	serverExec.SetOptionEnabled(KxPROCESS_SYNC_EVENTS, true);
	serverExec.SetOptionEnabled(KxPROCESS_WAIT_INPUT_IDLE, true);
	serverExec.SetOptionEnabled(KxPROCESS_WAIT_END, false);
	serverExec.Bind(KxEVT_PROCESS_IDLE, [clientInstance, &isRunSuccess](wxProcessEvent& event)
	{
		KIPCClient* client = new KIPCClient();
		client->InitConnection();
		
		*clientInstance = client;
		isRunSuccess = true;
	});
	serverExec.Run(KxPROCESS_RUN_SYNC);
	return isRunSuccess;
}

bool KIPCClient::CreateConnection()
{
	m_Connection = static_cast<KIPCConnection*>(MakeConnection(KIPC::GetHost(), KIPC::GetServiceName(), KIPC::GetTopic()));
	return IsConnected();
}
wxConnectionBase* KIPCClient::OnMakeConnection()
{
	wxLogInfo("Client: Connecting to server");
	return new KIPCConnection(this);
}
void KIPCClient::OnDisconnect()
{
	wxLogError("Client: Connection reset by server. Terminating");
	KLogEvent(T("VFSService.InstallFailed"), KLOG_CRITICAL).Send();
}

void KIPCClient::OnVFSStateChanged(const KIPCRequest_VFSStateChanged& config)
{
	KApp::Get().CallAfter([config]()
	{
		// Set mounted status
		KModManager::Get().SetMounted(config.IsEnabled());
		int status = config.GetStatus();

		// Send event before reloading 'KPluginManager'
		KVFSEvent event(config.IsEnabled());
		event.SetInt(status);
		event.Send();

		// Force load of plugin manager so plugin data will be loaded
		if (KVirtualFileSystemBase::IsSuccessCode(status) && config.IsEnabled())
		{
			if (KPluginManager* pluginManager = KPluginManager::GetInstance())
			{
				pluginManager->LoadIfNeeded();
			}
		}

		if (!KVirtualFileSystemBase::IsSuccessCode(status))
		{
			wxString message = wxString::Format("%s\r\n\r\n%s: %s", KVirtualFileSystemBase::GetStatusCodeMessage(status), T("VFS.MountPoint"), event.GetString());
			KLogEvent(message, KLOG_ERROR).Send();
		}

		KModManager::Get().DestroyMountStatusDialog();
	});
}

KIPCClient::KIPCClient()
{
}
KIPCClient::~KIPCClient()
{
	if (m_Connection)
	{
		m_Connection->StopAdvise(KIPCRequest_VFSStateChanged::GetClassName());
		m_Connection->Disconnect();
	}
}

bool KIPCClient::InitConnection()
{
	CreateConnection();
	if (m_Connection)
	{
		return m_Connection->StartAdvise(KIPCRequest_VFSStateChanged::GetClassName());
	}
	return false;
}
bool KIPCClient::Disconnect()
{
	if (m_Connection)
	{
		wxLogInfo("Client: Disconnecting from server");

		bool bRet = m_Connection->Disconnect();
		delete m_Connection;
		return bRet;
	}
	return false;
}
bool KIPCClient::InitVFSService()
{
	if (m_Connection)
	{
		return m_Connection->SendToServer(KIPCRequest_InitVFSService());
	}
	return false;
}
bool KIPCClient::UninstallVFSService()
{
	if (m_Connection)
	{
		return m_Connection->SendToServer(KIPCRequest_UninstallVFSService());
	}
	return false;
}

bool KIPCClient::CreateVFS_Mirror(const wxString& source, const wxString& target)
{
	if (m_Connection)
	{
		return m_Connection->SendToServer(KIPCRequest_CreateMirrorVFS(source, target));
	}
	return false;
}
bool KIPCClient::MirrorVFS_ClearList()
{
	if (m_Connection)
	{
		return m_Connection->SendToServer(KIPCRequest_ClearMirrorVFSList());
	}
	return false;
}

bool KIPCClient::CreateVFS_Convergence(const wxString& source, const wxString& writeTarget, const KxStringVector& virtualFolders, bool canDeleteInVirtualFolder)
{
	if (m_Connection)
	{
		bool isOK = false;
		isOK = m_Connection->SendToServer(KIPCRequest_CreateConvergenceVFS(source, writeTarget, canDeleteInVirtualFolder));
		for (const wxString& path: virtualFolders)
		{
			m_Connection->SendToServer(KIPCRequest_AddConvergenceVirtualFolder(path));
		}
		return isOK;
	}
	return false;
}
bool KIPCClient::ConvergenceVFS_ClearVirtualFolders()
{
	if (m_Connection)
	{
		return m_Connection->SendToServer(KIPCRequest_ClearConvergenceVirtualFolders());
	}
	return false;
}

bool KIPCClient::EnableVFS()
{
	if (m_Connection)
	{
		return m_Connection->SendToServer(KIPCRequest_EnableVFS(true));
	}
	return false;
}
bool KIPCClient::DisableVFS()
{
	if (m_Connection)
	{
		return m_Connection->SendToServer(KIPCRequest_EnableVFS(false));
	}
	return false;
}
