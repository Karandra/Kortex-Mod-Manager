#pragma once
#include "stdafx.h"
#include "VirtualFileSystem/IVFSService.h"
#include "VirtualFileSystem/IVirtualFileSystem.h"
#include <KxVirtualFileSystem/AbstractFS.h>
#include "MainApplicationLink.h"

namespace Kortex::VirtualFileSystem
{
	template<class T> class KxVFSWrapper: public IVirtualFileSystem, public T
	{
		private:
			FSController::MainApplicationLink* m_Link = nullptr;

		protected:
			void OnEnabled() override
			{
				m_Link->NotifyMounted(*this);
			}
			void OnDisabled() override
			{
				m_Link->NotifyUnmounted(*this);
			}

		public:
			template<class... Args> KxVFSWrapper(Args&&... arg)
				:T(*IVFSService::GetInstance()->GetNativeService<KxVFS::Service>(), std::forward<Args>(arg)...)
			{
				m_Link = FSController::MainApplicationLink::GetInstance();
			}

		public:
			IPC::FSHandle GetHandle() const override
			{
				return reinterpret_cast<IPC::FSHandle>(static_cast<const T*>(this));
			}

			bool IsEnabled() const override
			{
				return T::IsMounted();
			}
			void Enable() override
			{
				T::Mount();
			}
			void Disable() override
			{
				T::UnMount();
			}

		protected:
			NTSTATUS OnMount(KxVFS::EvtMounted& eventInfo) override
			{
				const NTSTATUS status = T::OnMount(eventInfo);
				OnEnabled();
				return status;
			}
			NTSTATUS OnUnMount(KxVFS::EvtUnMounted& eventInfo) override
			{
				const NTSTATUS status = T::OnUnMount(eventInfo);
				OnDisabled();
				return status;
			}
	};
}
