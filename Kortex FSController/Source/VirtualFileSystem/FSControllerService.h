#pragma once
#include "stdafx.h"
#include "Common.h"
#include "IPC/Message.h"
#include "VirtualFileSystem/IVirtualFileSystem.h"
#include "VirtualFileSystem/IVFSService.h"
#include <KxVirtualFileSystem/Service.h>
#include <KxVirtualFileSystem/AbstractFS.h>

namespace Kortex::FSController
{
	class RecievingWindow;
}

namespace Kortex::VirtualFileSystem
{
	class FSControllerService: public IVFSService, public KxVFS::Service
	{
		friend class FSController::RecievingWindow;

		private:
			FSController::RecievingWindow* m_RecievingWindow = nullptr;
			std::vector<std::unique_ptr<KxVFS::AbstractFS>> m_FileSystems;

		private:
			void OnMessage(const IPC::Message& message);
			IPC::FSHandle OnCreateFS(IPC::FileSystemID fileSystemID);

			void DestroyFS(KxVFS::AbstractFS& vfs);

		protected:
			void RegisterFS(IVirtualFileSystem& vfs) override
			{
			}
			void UnregisterFS(IVirtualFileSystem& vfs) override
			{
			}

		public:
			FSControllerService();
			virtual ~FSControllerService();

		public:
			bool IsOK() const override
			{
				return KxVFS::Service::IsOK();
			}
			void* GetNativeService() override
			{
				return static_cast<KxVFS::Service*>(this);
			}

			wxString GetServiceName() const override
			{
				return ToWxString(KxVFS::Service::GetServiceName());
			}
			bool IsInstalled() const override
			{
				return KxVFS::Service::IsInstalled();
			}
			bool IsStarted() const override
			{
				return KxVFS::Service::IsStarted();
			}
	
			bool Start()  override
			{
				return KxVFS::Service::Start();
			}
			bool Stop() override
			{
				return KxVFS::Service::Stop();
			}
			bool Install() override;
			bool Uninstall() override;

			void SetRecievingWindow(FSController::RecievingWindow* recievingWindow)
			{
				m_RecievingWindow = recievingWindow;
			}
			
			IPC::FSHandle GetFileSystemHandle(const KxVFS::AbstractFS& vfs) const;
			KxVFS::AbstractFS& GetFileSystemByHandle(IPC::FSHandle handle) const;

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
				return ToWxString(KxVFS::Service::GetLibraryVersion());
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
				return ToWxString(KxVFS::Service::GetDokanyVersion());
			}
	};
}
