#pragma once
#include "stdafx.h"
#include "Common.h"
#include "IPC/Message.h"
#include "VirtualFileSystem/IVirtualFileSystem.h"
#include "VirtualFileSystem/IVFSService.h"
#include <KxVirtualFileSystem/FileSystemService.h>
#include <KxVirtualFileSystem/IFileSystem.h>
#include <KxVirtualFileSystem/Logger/FileLogger.h>

namespace Kortex::FSController
{
	class RecievingWindow;
}

namespace Kortex::VirtualFileSystem
{
	class FSControllerService: public IVFSService, public KxVFS::FileSystemService
	{
		friend class FSController::RecievingWindow;

		private:
			FSController::RecievingWindow* m_RecievingWindow = nullptr;
			std::vector<std::unique_ptr<KxVFS::IFileSystem>> m_FileSystems;
			KxVFS::FileLogger m_Logger;

		private:
			void OnMessage(const IPC::Message& message);
			IPC::FSHandle CreateFS(IPC::FileSystemID fileSystemID);
			void DestroyFS(KxVFS::IFileSystem& fileSystem);

		protected:
			void RegisterFS(IVirtualFileSystem& fileSystem) override
			{
			}
			void UnregisterFS(IVirtualFileSystem& fileSystem) override
			{
			}

		public:
			FSControllerService();
			virtual ~FSControllerService();

		public:
			bool IsOK() const override
			{
				return KxVFS::FileSystemService::IsOK();
			}
			void* GetNativeService() override
			{
				return static_cast<KxVFS::FileSystemService*>(this);
			}

			wxString GetServiceName() const override
			{
				return ToWxString(KxVFS::FileSystemService::GetServiceName());
			}
			bool IsInstalled() const override
			{
				return KxVFS::FileSystemService::IsInstalled();
			}
			bool IsStarted() const override
			{
				return KxVFS::FileSystemService::IsStarted();
			}
	
			bool Start()  override
			{
				return KxVFS::FileSystemService::Start();
			}
			bool Stop() override
			{
				return KxVFS::FileSystemService::Stop();
			}
			bool Install() override;
			bool Uninstall() override;

			void SetRecievingWindow(FSController::RecievingWindow* recievingWindow)
			{
				m_RecievingWindow = recievingWindow;
			}
			
			IPC::FSHandle GetFileSystemHandle(const KxVFS::IFileSystem& fileSystem) const;
			KxVFS::IFileSystem& GetFileSystemByHandle(IPC::FSHandle handle) const;
			template<class T> T& GetFileSystemByHandle(IPC::FSHandle handle) const
			{
				return static_cast<T&>(GetFileSystemByHandle(handle));
			}

		public:
			wxString GetLibraryName() const override
			{
				return wxS("KxVirtualFileSystem");
			}
			wxString GetLibraryURL() const override
			{
				return wxS("https://github.com/dokan-dev/dokany");
			}
			KxVersion GetLibraryVersion() const override
			{
				return ToWxString(KxVFS::FileSystemService::GetLibraryVersion());
			}

			bool HasNativeLibrary() const override
			{
				return true;
			}
			wxString GetNativeLibraryName() const override
			{
				return wxS("Dokany");
			}
			wxString GetNativeLibraryURL() const override
			{
				return wxS("https://github.com/KerberX/KxVirtualFileSystem");
			}
			KxVersion GetNativeLibraryVersion() const override
			{
				return ToWxString(KxVFS::FileSystemService::GetDokanyVersion());
			}
			
		public:
			KxVFS::ILogger& GetLogger() override
			{
				return m_Logger;
			}
	};
}
