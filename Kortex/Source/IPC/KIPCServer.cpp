#include "stdafx.h"
#include "KIPCServer.h"
#include "KIPCConnection.h"
#include "KIPCRequest.h"
#include "KApp.h"
#include "VFS/KVirtualFileSystemService.h"
#include "VFS/KVirtualFileSystemMirror.h"
#include "VFS/KVirtualFileSystemConvergence.h"
#include <KxFramework/KxFile.h>

#if KIPC_SERVER
KIPCServer& KIPCServer::Get()
{
	return *KApp::Get().GetServer();
}
#endif
wxString KIPCServer::GetClientFileName()
{
	wxString path = "Kortex";
	if (KxSystem::Is64Bit())
	{
		path += " x64";
	}
	path += ".exe";
	return path;
}

wxConnectionBase* KIPCServer::OnAcceptConnection(const wxString& topic)
{
	if (topic == KIPC::GetTopic())
	{
		m_Connection = new KIPCConnection(this);
		return m_Connection;
	}
	return NULL;
}
void KIPCServer::OnDisconnect()
{
	KApp::Get().ExitApp();
}

void KIPCServer::OnInitService(const KIPCRequest_InitVFSService& config)
{
	m_Service = std::make_unique<KVirtualFileSystemService>();
	if (m_Service->Init())
	{
		if (!m_Service->IsInstalled())
		{
			// Notify install begins
			if (!m_Service->Install())
			{
				// Notify install failed
			}
		}
		else
		{
			// Reconfigure service
			m_Service->Install();
		}
		m_Service->Start();
	}
	else
	{
		// Init failed
	}
}
void KIPCServer::OnUninstallService(const KIPCRequest_UninstallVFSService& config)
{
	if (m_Service)
	{
		m_Service->Stop();
		m_Service->Uninstall();
	}
}
void KIPCServer::OnCreateConvergenceVFS(const KIPCRequest_CreateConvergenceVFS& config)
{
	m_Convergence = std::make_unique<KVirtualFileSystemConvergence>(GetServiceVFS(), config.GetMountPoint(), config.GetWriteTarget());
	m_Convergence->SetCanDeleteInVirtualFolder(config.CanDeleteInVirtualFolder());
	m_Convergence->Bind(KVFSEVT_UNMOUNTED, &KIPCServer::OnVFSUnmounted, this);

	// Make sure folders are exist
	KxFile(config.GetMountPoint()).CreateFolder();
	KxFile(config.GetWriteTarget()).CreateFolder();
}
void KIPCServer::OnAddConvergenceVirtualFolder(const KIPCRequest_AddConvergenceVirtualFolder& config)
{
	if (m_Convergence)
	{
		m_Convergence->AddVirtualFolder(config.GetPath());
	}
}
void KIPCServer::OnClearConvergenceVirtualFolders(const KIPCRequest_ClearConvergenceVirtualFolders& config)
{
	if (m_Convergence)
	{
		m_Convergence->ClearVirtualFolders();
	}
}
void KIPCServer::OnCreateMirrorVFS(const KIPCRequest_CreateMirrorVFS& config)
{
	auto& mirror = m_MirrorVFSList.emplace_back(std::make_unique<KVirtualFileSystemMirror>(GetServiceVFS(), config.GetTarget(), config.GetSource()));
	mirror->Bind(KVFSEVT_UNMOUNTED, &KIPCServer::OnVFSUnmounted, this);
}
void KIPCServer::OnClearMirrorVFSList(const KIPCRequest_ClearMirrorVFSList& config)
{
	m_MirrorVFSList.clear();
}
void KIPCServer::OnEnableVFS(const KIPCRequest_EnableVFS& config)
{
	config.ShouldEnable() ? EnableVFS() : DisableVFS();
}

void KIPCServer::OnVFSUnmounted(wxNotifyEvent& event)
{
	if (!m_ManualDisablingInProgress)
	{
		// Unmount all other VFS's
		DisableVFS();

		if (m_Connection)
		{
			m_Connection->SendToClient(KIPCRequest_VFSStateChanged(false, event.GetInt()));
		}
	}
}
void KIPCServer::ReportMountError(int code, KVirtualFileSystemBase* vfs)
{
	wxNotifyEvent event;
	event.SetEventObject(vfs);
	event.SetInt(code);
	event.SetString(vfs->GetMountPoint());
	OnVFSUnmounted(event);
}

KIPCServer::KIPCServer()
	:m_IsCreated(Create(KIPC::GetServiceName()))
{
}
KIPCServer::~KIPCServer()
{
	DisableVFS();
}

bool KIPCServer::IsVFSEnabled() const
{
	if (m_Convergence && m_Convergence->IsMounted())
	{
		return true;
	}
	for (auto& mirror: m_MirrorVFSList)
	{
		if (mirror->IsMounted())
		{
			return true;
		}
	}
	return false;
}
int KIPCServer::EnableVFS()
{
	if (!IsVFSEnabled())
	{
		int status = KVirtualFileSystemBase::GetSuccessCode();
		if (m_Convergence && !m_Convergence->IsMounted())
		{
			status = m_Convergence->Mount();
			if (!KVirtualFileSystemBase::IsSuccessCode(status))
			{
				ReportMountError(status, m_Convergence.get());
				return status;
			}
		}

		if (KVirtualFileSystemBase::IsSuccessCode(status))
		{
			for (auto& mirror: m_MirrorVFSList)
			{
				if (!mirror->IsMounted())
				{
					KxFile(mirror->GetSource()).CreateFolder();
					KxFile(mirror->GetMountPoint()).CreateFolder();
					status = mirror->Mount();

					if (!KVirtualFileSystemBase::IsSuccessCode(status))
					{
						ReportMountError(status, mirror.get());
						return status;
					}
				}
			}
		}

		// Notify client
		if (m_Connection)
		{
			m_Connection->SendToClient(KIPCRequest_VFSStateChanged(IsVFSEnabled(), status));
		}
		return status;
	}
	else
	{
		return KVFS_STATUS_NOT_STARTED;
	}
}
bool KIPCServer::DisableVFS()
{
	m_ManualDisablingInProgress = true;

	bool b1 = false;
	if (m_Convergence)
	{
		b1 = m_Convergence->UnMount();
	}

	bool b2 = false;
	for (auto& mirror: m_MirrorVFSList)
	{
		b2 = mirror->UnMount();
	}

	m_ManualDisablingInProgress = false;
	return b1 && b2;
}
