#pragma once
#include "stdafx.h"
#include "IPC/Common.h"
#include "VirtualFileSystem/IVFSService.h"

namespace Kortex
{
	class IVirtualFileSystem
	{
		protected:
			virtual void OnEnabled() = 0;
			virtual void OnDisabled() = 0;

		public:
			IVirtualFileSystem()
			{
				if (IVFSService* service = IVFSService::GetInstance())
				{
					service->RegisterFS(*this);
				}
			}
			virtual ~IVirtualFileSystem()
			{
				if (IVFSService* service = IVFSService::GetInstance())
				{
					service->UnregisterFS(*this);
				}
			}

		public:
			virtual IPC::FSHandle GetHandle() const = 0;

			virtual bool IsEnabled() const = 0;
			virtual void Enable() = 0;
			virtual void Disable() = 0;
	};
}
