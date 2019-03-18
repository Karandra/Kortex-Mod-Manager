#pragma once
#include "stdafx.h"
#include "IPC/MainApplication.h"
#include <KxFramework/KxSingleton.h>

namespace KxVFS
{
	class IFileSystem;
}
namespace Kortex
{
	class IVirtualFileSystem;
}
namespace Kortex::VirtualFileSystem
{
	class FSControllerService;
}

namespace Kortex::FSController
{
	class RecievingWindow;

	class MainApplicationLink: public IPC::MainApplication, public KxSingletonPtr<MainApplicationLink>
	{
		private:
			VirtualFileSystem::FSControllerService* m_Service = nullptr;
			RecievingWindow* m_RecievingWindow = nullptr;

		public:
			MainApplicationLink(HWND windowHandle);
			virtual ~MainApplicationLink();

		public:
			void SetService(VirtualFileSystem::FSControllerService& service)
			{
				m_Service = &service;
			}
			void SetRecievingWindow(FSController::RecievingWindow* recievingWindow)
			{
				m_RecievingWindow = recievingWindow;
			}

			void NotifyMounted(const KxVFS::IFileSystem& fileSystem);
			void NotifyUnmounted(const KxVFS::IFileSystem& fileSystem);
	};
}
