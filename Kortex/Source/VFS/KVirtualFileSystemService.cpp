#include "stdafx.h"
#include "KVirtualFileSystemService.h"
#include "KApp.h"
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

wxString KVirtualFileSystemService::GetLibraryVersion()
{
	return KxVFSService::GetLibraryVersion();
}
wxString KVirtualFileSystemService::GetDokanVersion()
{
	return KxVFSService::GetDokanVersion();
}

wxString KVirtualFileSystemService::InitLibraryPath()
{
	wxString path = KApp::Get().GetDataFolder() + "\\VFS\\";
	#if defined _WIN64
	path += "KxVirtualFileSystem x64.dll";
	#else
	path += "KxVirtualFileSystem.dll";
	#endif
	return path;
}
wxString KVirtualFileSystemService::InitDriverPath()
{
	wxString path = KApp::Get().GetDataFolder() + "\\VFS\\Drivers\\";
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

void KVirtualFileSystemService::UnInit()
{
	delete m_ServiceImpl;
	m_ServiceImpl = NULL;
}

KVirtualFileSystemService::KVirtualFileSystemService()
	:m_Name("KortexVFS"),
	m_DisplayName(wxString::Format("%s VFS Service", KApp::Get().GetAppDisplayName())),
	m_Description(wxString::Format("The VFS service provides support for a virtual file system for %s", KApp::Get().GetAppDisplayName())),
	m_LibraryPath(InitLibraryPath()),
	m_DriverPath(InitDriverPath())
{
	if (!m_LibraryPath.IsEmpty())
	{
		m_LibraryHandle = ::LoadLibraryW(m_LibraryPath);
	}
}
KVirtualFileSystemService::~KVirtualFileSystemService()
{
	UnInit();

	if (m_LibraryHandle)
	{
		::FreeLibrary(m_LibraryHandle);
	}
}
bool KVirtualFileSystemService::Init()
{
	if (m_ServiceImpl == NULL)
	{
		m_ServiceImpl = new KxVFSService(m_Name);
		return m_ServiceImpl->IsOK();
	}
	return false;
}

bool KVirtualFileSystemService::IsOK() const
{
	return m_LibraryHandle != NULL && !m_DriverPath.IsEmpty();
}

KxVFSService* KVirtualFileSystemService::GetServiceImpl() const
{
	return m_ServiceImpl;
}
bool KVirtualFileSystemService::IsReady() const
{
	return IsOK() && (m_ServiceImpl != NULL && m_ServiceImpl->IsStarted());
}
bool KVirtualFileSystemService::IsStarted() const
{
	return m_ServiceImpl->IsStarted();
}
bool KVirtualFileSystemService::IsInstalled() const
{
	return m_ServiceImpl->IsInstalled();
}

bool KVirtualFileSystemService::Start()
{
	return m_ServiceImpl->Start();
}
bool KVirtualFileSystemService::Stop()
{
	return m_ServiceImpl->Stop();
}
bool KVirtualFileSystemService::Install()
{
	return m_ServiceImpl->Install(m_DriverPath, m_DisplayName, m_Description);
}
bool KVirtualFileSystemService::Uninstall()
{
	return m_ServiceImpl->Uninstall();
}
