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

	void MainApplicationLink::NotifyMounted(const KxVFS::AbstractFS& vfs)
	{
		Send(RequestID::FSEnabled, m_Service->GetFileSystemHandle(vfs));
	}
	void MainApplicationLink::NotifyUnmounted(const KxVFS::AbstractFS& vfs)
	{
		Send(RequestID::FSDisabled, m_Service->GetFileSystemHandle(vfs));
	}
}
