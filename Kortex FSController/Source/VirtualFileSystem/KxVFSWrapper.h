#pragma once
#include "stdafx.h"
#include "VirtualFileSystem/IVFSService.h"
#include "VirtualFileSystem/IVirtualFileSystem.h"
#include <KxVirtualFileSystem/IFileSystem.h>
#include "MainApplicationLink.h"

namespace Kortex::VirtualFileSystem
{
	template<class T> class KxVFSWrapper: public IVirtualFileSystem, public T
	{
		public:
			using TBase = T;
			using TWrapper = KxVFSWrapper;

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
				:TBase(*IVFSService::GetInstance()->GetNativeService<KxVFS::FileSystemService>(), std::forward<Args>(arg)...)
			{
				m_Link = FSController::MainApplicationLink::GetInstance();
			}

		public:
			IPC::FSHandle GetHandle() const override
			{
				return reinterpret_cast<IPC::FSHandle>(static_cast<const TBase*>(this));
			}

			wxString GetMountPointLocation() const
			{
				return ToWxString(TBase::GetMountPoint());
			}
			void SetMountPointLocation(const wxString& mountPoint)
			{
				TBase::SetMountPoint(ToKxDynamicStringRef(mountPoint));
			}

			bool IsEnabled() const override
			{
				return TBase::IsMounted();
			}
			bool Enable() override
			{
				return TBase::Mount().IsSuccess();
			}
			void Disable() override
			{
				TBase::UnMount();
			}

		protected:
			NTSTATUS OnMount(KxVFS::EvtMounted& eventInfo) override
			{
				const NTSTATUS status = TBase::OnMount(eventInfo);
				OnEnabled();
				return status;
			}
			NTSTATUS OnUnMount(KxVFS::EvtUnMounted& eventInfo) override
			{
				const NTSTATUS status = TBase::OnUnMount(eventInfo);
				OnDisabled();
				return status;
			}
	};
}
