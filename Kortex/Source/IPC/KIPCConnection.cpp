#include "stdafx.h"
#include "KIPCConnection.h"
#include "KIPCRequest.h"
#include "KIPCServer.h"
#include "KIPCClient.h"
#include "VFS/KVirtualFileSystemConvergence.h"
#include "VFS/KVirtualFileSystemMirror.h"

bool KIPCConnection::OnDisconnect()
{
	if constexpr(IsServer())
	{
		GetServer()->OnDisconnect();
	}
	else
	{
		GetClient()->OnDisconnect();
	}
	return wxConnection::OnDisconnect();
}
bool KIPCConnection::OnPoke(const wxString& topic, const wxString& item, const void* data, size_t nSize, wxIPCFormat format)
{
	if constexpr(IsServer())
	{
		if (auto service = ReceiveRequest<KIPCRequestNS::InitVFSService>(item, data, nSize))
		{
			GetServer()->OnInitService(*service);
			return true;
		}
		else if (auto uninstall = ReceiveRequest<KIPCRequestNS::UninstallVFSService>(item, data, nSize))
		{
			GetServer()->OnUninstallService(*uninstall);
			return true;
		}
		else if (auto enable = ReceiveRequest<KIPCRequestNS::EnableVFS>(item, data, nSize))
		{
			GetServer()->OnEnableVFS(*enable);
			return true;
		}
		else if (auto mirror = ReceiveRequest<KIPCRequestNS::CreateMirrorVFS>(item, data, nSize))
		{
			GetServer()->OnCreateMirrorVFS(*mirror);
			return true;
		}
		else if (auto mirrorClearList = ReceiveRequest<KIPCRequestNS::ClearMirrorVFSList>(item, data, nSize))
		{
			GetServer()->OnClearMirrorVFSList(*mirrorClearList);
			return true;
		}
		else if (auto convergence = ReceiveRequest<KIPCRequestNS::CreateConvergenceVFS>(item, data, nSize))
		{
			GetServer()->OnCreateConvergenceVFS(*convergence);
			return true;
		}
		else if (auto convergenceAddFolder = ReceiveRequest<KIPCRequestNS::AddConvergenceVirtualFolder>(item, data, nSize))
		{
			GetServer()->OnAddConvergenceVirtualFolder(*convergenceAddFolder);
			return true;
		}
		else if (auto convergenceClearFolders = ReceiveRequest<KIPCRequestNS::ClearConvergenceVirtualFolders>(item, data, nSize))
		{
			GetServer()->OnClearConvergenceVirtualFolders(*convergenceClearFolders);
			return true;
		}
	}
	return false;
}
bool KIPCConnection::OnAdvise(const wxString& topic, const wxString& item, const void* data, size_t nSize, wxIPCFormat format)
{
	if constexpr(IsClient())
	{
		if (auto stateChanged = ReceiveRequest<KIPCRequestNS::VFSStateChanged>(item, data, nSize))
		{
			GetClient()->OnVFSStateChanged(*stateChanged);
			return true;
		}
	}
	return false;
}
bool KIPCConnection::OnStartAdvise(const wxString& topic, const wxString& item)
{
	if (topic == KIPC::GetTopic())
	{
		return true;
	}
	return false;
}
bool KIPCConnection::OnStopAdvise(const wxString& topic, const wxString& item)
{
	return OnStartAdvise(topic, item);
}

KIPCConnection::KIPCConnection(KIPCServer* server)
	:m_Server(server)
{
}
KIPCConnection::KIPCConnection(KIPCClient* client)
	:m_Client(client)
{
}
KIPCConnection::~KIPCConnection()
{
}
