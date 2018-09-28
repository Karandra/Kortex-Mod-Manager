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
bool KIPCConnection::OnPoke(const wxString& topic, const wxString& item, const void* data, size_t size, wxIPCFormat format)
{
	if constexpr(IsServer())
	{
		if (auto service = ReceiveRequest<KIPCRequestNS::InitVFSService>(item, data, size))
		{
			GetServer()->OnInitService(*service);
			return true;
		}
		if (auto uninstall = ReceiveRequest<KIPCRequestNS::UninstallVFSService>(item, data, size))
		{
			GetServer()->OnUninstallService(*uninstall);
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		if (auto enable = ReceiveRequest<KIPCRequestNS::EnableVFS>(item, data, size))
		{
			GetServer()->OnEnableVFS(*enable);
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		if (auto mirror = ReceiveRequest<KIPCRequestNS::CreateMirrorVFS>(item, data, size))
		{
			GetServer()->OnCreateMirrorVFS(*mirror);
			return true;
		}
		if (auto mirrorClearList = ReceiveRequest<KIPCRequestNS::ClearMirrorVFSList>(item, data, size))
		{
			GetServer()->OnClearMirrorVFSList(*mirrorClearList);
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		if (auto convergence = ReceiveRequest<KIPCRequestNS::CreateConvergenceVFS>(item, data, size))
		{
			GetServer()->OnCreateConvergenceVFS(*convergence);
			return true;
		}
		if (auto convergenceAddFolder = ReceiveRequest<KIPCRequestNS::AddConvergenceVirtualFolder>(item, data, size))
		{
			GetServer()->OnAddConvergenceVirtualFolder(*convergenceAddFolder);
			return true;
		}
		if (auto convergenceClearFolders = ReceiveRequest<KIPCRequestNS::ClearConvergenceVirtualFolders>(item, data, size))
		{
			GetServer()->OnClearConvergenceVirtualFolders(*convergenceClearFolders);
			return true;
		}
		if (auto convergenceBuildIndex = ReceiveRequest<KIPCRequestNS::BuildConvergenceIndex>(item, data, size))
		{
			GetServer()->OnBuildConvergenceIndex(*convergenceBuildIndex);
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		if (auto convergenceBeginIndex = ReceiveRequest<KIPCRequestNS::BeginConvergenceIndex>(item, data, size))
		{
			GetServer()->OnBeginConvergenceIndex(*convergenceBeginIndex);
			return true;
		}
		if (auto convergenceCommitIndex = ReceiveRequest<KIPCRequestNS::CommitConvergenceIndex>(item, data, size))
		{
			GetServer()->OnCommitConvergenceIndex(*convergenceCommitIndex);
			return true;
		}
		if (auto convergenceAddIndex = ReceiveRequest<KIPCRequestNS::AddConvergenceIndex>(item, data, size))
		{
			GetServer()->OnAddConvergenceIndex(*convergenceAddIndex);
			return true;
		}
	}
	return false;
}
bool KIPCConnection::OnAdvise(const wxString& topic, const wxString& item, const void* data, size_t size, wxIPCFormat format)
{
	if constexpr(IsClient())
	{
		if (auto stateChanged = ReceiveRequest<KIPCRequestNS::VFSStateChanged>(item, data, size))
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
