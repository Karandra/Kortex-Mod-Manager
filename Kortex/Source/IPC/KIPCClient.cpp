#include "stdafx.h"
#include "KIPCClient.h"
#include "KIPCConnection.h"
#include <Kortex/Application.hpp>
#include <Kortex/ModManager.hpp>
#include <Kortex/Events.hpp>
#include "VFS/KVFSService.h"
#include <KxFramework/KxSystem.h>
#include <KxFramework/KxProcess.h>
#include <KxFramework/KxProgressDialog.h>

using namespace Kortex;
using namespace Kortex::ModManager;

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
	Kortex::LogEvent(KTr("VFSService.InstallFailed"), Kortex::LogLevel::Critical).Send();
}

void KIPCClient::OnAcceptRequest(const KIPCRequestNS::VFSStateChanged& config)
{
	IEvent::CallAfter([config]()
	{
		// Set mounted status
		Kortex::IModManager::GetInstance()->SetMounted(config.IsEnabled());
		int status = config.GetStatus();

		// Send event before reloading 'IPluginManager'
		VirtualFileSystemEvent event(Events::VirtualFileSystemToggled, config.IsEnabled());
		event.SetInt(status);
		event.Send();

		if (!KVFSService::IsSuccessCode(status))
		{
			wxString message = wxString::Format("%s\r\n\r\n%s: %s", KVFSService::GetStatusCodeMessage(status), KTr("VFS.MountPoint"), event.GetString());
			Kortex::LogEvent(message, Kortex::LogLevel::Error).Send();
		}

		//IModManager::GetInstance()->DestroyMountStatusDialog();
	});
}

KIPCClient::KIPCClient()
{
}
KIPCClient::~KIPCClient()
{
	if (m_Connection)
	{
		m_Connection->StopAdvise(KIPCRequestNS::VFSStateChanged::GetClassTypeName());
		m_Connection->Disconnect();
	}
}

bool KIPCClient::InitConnection()
{
	CreateConnection();
	if (m_Connection)
	{
		return m_Connection->StartAdvise(KIPCRequestNS::VFSStateChanged::GetClassTypeName());
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
		return m_Connection->SendToServer(KIPCRequestNS::InitVFSService());
	}
	return false;
}
bool KIPCClient::UninstallVFSService()
{
	if (m_Connection)
	{
		return m_Connection->SendToServer(KIPCRequestNS::UninstallVFSService());
	}
	return false;
}

bool KIPCClient::CreateVFS_Mirror(const wxString& source, const wxString& target)
{
	if (m_Connection)
	{
		return m_Connection->SendToServer(KIPCRequestNS::CreateMirrorVFS(source, target));
	}
	return false;
}
bool KIPCClient::CreateVFS_MultiMirror(const KxStringVector& sources, const wxString& target)
{
	if (m_Connection)
	{
		return m_Connection->SendToServer(KIPCRequestNS::CreateMultiMirrorVFS(sources, target));
	}
	return false;
}
bool KIPCClient::MirrorVFS_ClearList()
{
	if (m_Connection)
	{
		return m_Connection->SendToServer(KIPCRequestNS::ClearMirrorVFSList());
	}
	return false;
}

bool KIPCClient::CreateVFS_Convergence(const wxString& source, const wxString& writeTarget, const KxStringVector& virtualFolders, bool canDeleteInVirtualFolder)
{
	if (m_Connection)
	{
		bool isOK = false;
		isOK = m_Connection->SendToServer(KIPCRequestNS::CreateConvergenceVFS(source, writeTarget, canDeleteInVirtualFolder));
		for (const wxString& path: virtualFolders)
		{
			m_Connection->SendToServer(KIPCRequestNS::AddConvergenceVirtualFolder(path));
		}
		return isOK;
	}
	return false;
}
bool KIPCClient::ConvergenceVFS_ClearVirtualFolders()
{
	if (m_Connection)
	{
		return m_Connection->SendToServer(KIPCRequestNS::ClearConvergenceVirtualFolders());
	}
	return false;
}
bool KIPCClient::ConvergenceVFS_BuildDispatcherIndex()
{
	if (m_Connection)
	{
		return m_Connection->SendToServer(KIPCRequestNS::BuildConvergenceIndex());
	}
	return false;
}
bool KIPCClient::ConvergenceVFS_SetDispatcherIndex()
{
	if (m_Connection)
	{
		if (m_Connection->SendToServer(KIPCRequestNS::BeginConvergenceIndex()))
		{
			IModDispatcher::GetInstance()->GetVirtualTree().WalkTree([this](const FileTreeNode& node)
			{
				if (node.GetMod().IsActive())
				{
					return m_Connection->SendToServer(KIPCRequestNS::AddConvergenceIndex(node.GetRelativePath(), node.GetFullPath()));
				}
				return true;
			});
			return m_Connection->SendToServer(KIPCRequestNS::CommitConvergenceIndex());
		}
	}
	return false;
}

bool KIPCClient::ToggleVFS()
{
	if (m_Connection)
	{
		return m_Connection->SendToServer(KIPCRequestNS::ToggleVFS(true));
	}
	return false;
}
bool KIPCClient::DisableVFS()
{
	if (m_Connection)
	{
		return m_Connection->SendToServer(KIPCRequestNS::ToggleVFS(false));
	}
	return false;
}
