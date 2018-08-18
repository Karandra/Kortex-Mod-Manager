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
		if (auto service = ReceiveRequest<KIPCRequest_InitVFSService>(item, data, nSize))
		{
			GetServer()->OnInitService(*service);
			return true;
		}
		else if (auto pUninstall = ReceiveRequest<KIPCRequest_UninstallVFSService>(item, data, nSize))
		{
			GetServer()->OnUninstallService(*pUninstall);
			return true;
		}
		else if (auto pEnable = ReceiveRequest<KIPCRequest_EnableVFS>(item, data, nSize))
		{
			GetServer()->OnEnableVFS(*pEnable);
			return true;
		}
		else if (auto mirror = ReceiveRequest<KIPCRequest_CreateMirrorVFS>(item, data, nSize))
		{
			GetServer()->OnCreateMirrorVFS(*mirror);
			return true;
		}
		else if (auto pMirrorClearList = ReceiveRequest<KIPCRequest_ClearMirrorVFSList>(item, data, nSize))
		{
			GetServer()->OnClearMirrorVFSList(*pMirrorClearList);
			return true;
		}
		else if (auto pConvergence = ReceiveRequest<KIPCRequest_CreateConvergenceVFS>(item, data, nSize))
		{
			GetServer()->OnCreateConvergenceVFS(*pConvergence);
			return true;
		}
		else if (auto pConvergenceAddFolder = ReceiveRequest<KIPCRequest_AddConvergenceVirtualFolder>(item, data, nSize))
		{
			GetServer()->OnAddConvergenceVirtualFolder(*pConvergenceAddFolder);
			return true;
		}
		else if (auto pConvergenceClearFolders = ReceiveRequest<KIPCRequest_ClearConvergenceVirtualFolders>(item, data, nSize))
		{
			GetServer()->OnClearConvergenceVirtualFolders(*pConvergenceClearFolders);
			return true;
		}
	}
	return false;
}
bool KIPCConnection::OnAdvise(const wxString& topic, const wxString& item, const void* data, size_t nSize, wxIPCFormat format)
{
	if constexpr(IsClient())
	{
		if (auto stateChanged = ReceiveRequest<KIPCRequest_VFSStateChanged>(item, data, nSize))
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
