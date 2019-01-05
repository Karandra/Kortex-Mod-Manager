#pragma once
#include "stdafx.h"
#include "VirtualFileSystem/IVFSService.h"
#include "VirtualFileSystem/IVirtualFileSystem.h"
#include "IPC/FSController.h"

namespace Kortex::VirtualFileSystem
{
	class AbstractFS: public IVirtualFileSystem
	{
		protected:
			IPC::FSController& m_Controller;
			IPC::FSHandle m_Handle = 0;

		protected:
			void OnEnabled() override;
			void OnDisabled() override;

		public:
			AbstractFS(IPC::FileSystemID id);

		public:
			IPC::FSHandle GetHandle() const override;

			bool IsEnabled() const override;
			void Enable() override;
			void Disable() override;

		public:
			void SetMountPoint(const wxString& path);
	};
}
