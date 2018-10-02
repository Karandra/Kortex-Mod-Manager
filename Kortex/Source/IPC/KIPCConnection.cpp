#include "stdafx.h"
#include "KIPCConnection.h"
#include "KIPCRequest.h"
#include "KIPCClient.h"
#include "KIPCServer.h"
#include "VFS/KVFSConvergence.h"
#include "VFS/KVFSMirror.h"

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
	using TypeID = KIPCRequest::TypeID;

	if constexpr(IsServer())
	{
		switch (TestRequest(item, data, size))
		{
			//////////////////////////////////////////////////////////////////////////
			case TypeID::InitVFSService:
			{
				GetServer()->OnAcceptRequest(ReceiveRequest<KIPCRequestNS::InitVFSService>(data));
				break;
			}
			case TypeID::UninstallVFSService:
			{
				GetServer()->OnAcceptRequest(ReceiveRequest<KIPCRequestNS::UninstallVFSService>(data));
				break;
			}

			//////////////////////////////////////////////////////////////////////////
			case TypeID::ToggleVFS:
			{
				GetServer()->OnAcceptRequest(ReceiveRequest<KIPCRequestNS::ToggleVFS>(data));
				break;
			}

			//////////////////////////////////////////////////////////////////////////
			case TypeID::CreateMirrorVFS:
			{
				GetServer()->OnAcceptRequest(ReceiveRequest<KIPCRequestNS::CreateMirrorVFS>(data));
				break;
			}
			case TypeID::CreateMultiMirrorVFS:
			{
				GetServer()->OnAcceptRequest(ReceiveRequest<KIPCRequestNS::CreateMultiMirrorVFS>(data));
				break;
			}
			case TypeID::ClearMirrorVFSList:
			{
				GetServer()->OnAcceptRequest(ReceiveRequest<KIPCRequestNS::ClearMirrorVFSList>(data));
				break;
			}
			
			//////////////////////////////////////////////////////////////////////////
			case TypeID::CreateConvergenceVFS:
			{
				GetServer()->OnAcceptRequest(ReceiveRequest<KIPCRequestNS::CreateConvergenceVFS>(data));
				break;
			}
			case TypeID::AddConvergenceVirtualFolder:
			{
				GetServer()->OnAcceptRequest(ReceiveRequest<KIPCRequestNS::AddConvergenceVirtualFolder>(data));
				break;
			}
			case TypeID::ClearConvergenceVirtualFolders:
			{
				GetServer()->OnAcceptRequest(ReceiveRequest<KIPCRequestNS::ClearConvergenceVirtualFolders>(data));
				break;
			}
			case TypeID::BuildConvergenceIndex:
			{
				GetServer()->OnAcceptRequest(ReceiveRequest<KIPCRequestNS::BuildConvergenceIndex>(data));
				break;
			}

			case TypeID::BeginConvergenceIndex:
			{
				GetServer()->OnAcceptRequest(ReceiveRequest<KIPCRequestNS::BeginConvergenceIndex>(data));
				break;
			}
			case TypeID::CommitConvergenceIndex:
			{
				GetServer()->OnAcceptRequest(ReceiveRequest<KIPCRequestNS::CommitConvergenceIndex>(data));
				break;
			}
			case TypeID::AddConvergenceIndex:
			{
				GetServer()->OnAcceptRequest(ReceiveRequest<KIPCRequestNS::AddConvergenceIndex>(data));
				break;
			}

			default:
			{
				return false;
			}
		};
		return true;
	}
	return false;
}
bool KIPCConnection::OnAdvise(const wxString& topic, const wxString& item, const void* data, size_t size, wxIPCFormat format)
{
	using TypeID = KIPCRequest::TypeID;

	if constexpr (IsClient())
	{
		switch (TestRequest(item, data, size))
		{
			case TypeID::VFSStateChanged:
			{
				GetClient()->OnAcceptRequest(ReceiveRequest<KIPCRequestNS::VFSStateChanged>(data));
				break;
			}

			default:
			{
				return false;
			}
		};
		return true;
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
