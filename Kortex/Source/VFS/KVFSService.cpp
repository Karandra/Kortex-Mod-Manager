#include "stdafx.h"
#include "KVFSService.h"
#include <Kortex/Application.hpp>
#include <KxFramework/KxSystem.h>
#include <KxFramework/KxLibrary.h>
#include <KxFramework/KxFile.h>

#include "KxVFSService.h"
#include "KxVFSBase.h"
#if defined _WIN64
#pragma comment(lib, "KxVirtualFileSystem x64.lib")
#else
#pragma comment(lib, "KxVirtualFileSystem.lib")
#endif

wxDEFINE_EVENT(KEVT_VFS_MOUNTED, KxBroadcastEvent);
wxDEFINE_EVENT(KEVT_VFS_UNMOUNTED, KxBroadcastEvent);

//////////////////////////////////////////////////////////////////////////
wxString KVFSService::GetLibraryVersion()
{
	return KxVFSService::GetLibraryVersion();
}
wxString KVFSService::GetDokanyVersion()
{
	return KxVFSService::GetDokanyVersion();
}

int KVFSService::GetSuccessCode()
{
	return DOKAN_SUCCESS;
}
bool KVFSService::IsSuccessCode(int code)
{
	return DOKAN_SUCCEEDED(code);
}
wxString KVFSService::GetStatusCodeMessage(int code)
{
	if (code != KVFS_STATUS_NOT_STARTED && (code < KVFS_STATUS_MIN || code > KVFS_STATUS_MAX))
	{
		code = KVFS_STATUS_UNKNOWN_CODE;
	}
	#if KIPC_SERVER
	return wxString::Format("%d", (int)std::abs(code));
	#else
	return KTr(wxString::Format("VFS.Error%d", (int)std::abs(code)));
	#endif
}

wxString KVFSService::InitLibraryPath()
{
	wxString path = Kortex::IApplication::GetInstance()->GetDataFolder() + "\\VFS\\";
	#if defined _WIN64
	path += "KxVirtualFileSystem x64.dll";
	#else
	path += "KxVirtualFileSystem.dll";
	#endif
	return path;
}
wxString KVFSService::InitDriverPath()
{
	wxString path = Kortex::IApplication::GetInstance()->GetDataFolder() + "\\VFS\\Drivers\\";
	if (KxSystem::IsWindows10OrGreater())
	{
		path += "Win10";
	}
	else if (KxSystem::IsWindows8Point1OrGreater())
	{
		path += "Win8.1";
	}
	else if (KxSystem::IsWindows8OrGreater())
	{
		path += "Win8";
	}
	else
	{
		path += "Win7";
	}

	if (KxSystem::Is64Bit())
	{
		path += " x64";
	}
	path += "\\dokan2.sys";

	KxFile filePath(path);
	if (filePath.IsFileExist())
	{
		return filePath.GetFullPath();
	}
	return wxEmptyString;
}

void KVFSService::UnInit()
{
	delete m_ServiceImpl;
	m_ServiceImpl = nullptr;
}

KVFSService::KVFSService()
	:m_Name("KortexVFS"),
	m_DisplayName(KxString::Format("%1 VFS Service", Kortex::IApplication::GetInstance()->GetName())),
	m_Description(KxString::Format("The VFS service provides support for a virtual file system for %1", Kortex::IApplication::GetInstance()->GetName())),
	m_LibraryPath(InitLibraryPath()),
	m_DriverPath(InitDriverPath())
{
	if (!m_LibraryPath.IsEmpty())
	{
		m_LibraryHandle = ::LoadLibraryW(m_LibraryPath.wc_str());
	}
}
KVFSService::~KVFSService()
{
	UnInit();

	if (m_LibraryHandle)
	{
		::FreeLibrary(m_LibraryHandle);
	}
}
bool KVFSService::Init()
{
	if (m_ServiceImpl == nullptr)
	{
		m_ServiceImpl = new KxVFSService(m_Name.wc_str());
		return m_ServiceImpl->IsOK();
	}
	return false;
}

bool KVFSService::IsOK() const
{
	return m_LibraryHandle != nullptr && !m_DriverPath.IsEmpty();
}

KxVFSService* KVFSService::GetServiceImpl() const
{
	return m_ServiceImpl;
}
bool KVFSService::IsReady() const
{
	return IsOK() && (m_ServiceImpl != nullptr && m_ServiceImpl->IsStarted());
}
bool KVFSService::IsStarted() const
{
	return m_ServiceImpl->IsStarted();
}
bool KVFSService::IsInstalled() const
{
	return m_ServiceImpl->IsInstalled();
}

bool KVFSService::Start()
{
	return m_ServiceImpl->Start();
}
bool KVFSService::Stop()
{
	return m_ServiceImpl->Stop();
}
bool KVFSService::Install()
{
	return m_ServiceImpl->Install(m_DriverPath.wc_str(), m_DisplayName.wc_str(), m_Description.wc_str());
}
bool KVFSService::Uninstall()
{
	return m_ServiceImpl->Uninstall();
}
