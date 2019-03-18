#include "stdafx.h"
#include "MainApplicationLink.h"
#include "RecievingWindow.h"
#include "VirtualFileSystem/FSControllerService.h"

using namespace Kortex::IPC;

namespace Kortex::FSController
{
	MainApplicationLink::MainApplicationLink(HWND windowHandle)
		:MainApplication(windowHandle)
	{
	}
	MainApplicationLink::~MainApplicationLink()
	{
	}

	void MainApplicationLink::NotifyMounted(const KxVFS::IFileSystem& fileSystem)
	{
		Send(RequestID::FSEnabled, m_Service->GetFileSystemHandle(fileSystem));
	}
	void MainApplicationLink::NotifyUnmounted(const KxVFS::IFileSystem& fileSystem)
	{
		Send(RequestID::FSDisabled, m_Service->GetFileSystemHandle(fileSystem));
	}
}
