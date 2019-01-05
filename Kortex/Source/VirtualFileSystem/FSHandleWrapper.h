#pragma once
#include "stdafx.h"
#include "VirtualFileSystem/IVFSService.h"
#include "VirtualFileSystem/IVirtualFileSystem.h"
#include "IPC/FSController.h"

namespace Kortex::VirtualFileSystem
{
	class FSHandleWrapper: public IVirtualFileSystem
	{
		protected:
			IPC::FSController& m_Contrller;
			IPC::FSHandle m_Handle = 0;

		protected:
			void OnEnabled() override
			{
			}
			void OnDisabled() override
			{
			}

		public:
			FSHandleWrapper(IPC::FileSystemID id)
				:m_Contrller(*IVFSService::GetInstance()->GetNativeService<IPC::FSController>())
			{
				m_Contrller.Send(IPC::RequestID::CreateFS, id).GetAs(m_Handle);
			}

		public:
			IPC::FSHandle GetHandle() const override
			{
				return m_Handle;
			}

			bool IsEnabled() const override
			{
				return m_Contrller.Send(IPC::RequestID::FSIsEnabled, m_Handle).GetAs<bool>();
			}
			void Enable() override
			{
				m_Contrller.Send(IPC::RequestID::FSEnable, m_Handle);
			}
			void Disable() override
			{
				m_Contrller.Send(IPC::RequestID::FSDisable, m_Handle);
			}
	};
}
