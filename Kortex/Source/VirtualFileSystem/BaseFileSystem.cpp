#include "stdafx.h"
#include "BaseFileSystem.h"
#include "IPC/Common.h"

using Kortex::IPC::RequestID;

namespace Kortex::VirtualFileSystem
{
	void BaseFileSystem::OnEnabled()
	{
	}
	void BaseFileSystem::OnDisabled()
	{
	}

	BaseFileSystem::BaseFileSystem(IPC::FileSystemID id)
		:m_Controller(*IVFSService::GetInstance()->GetNativeService<IPC::FSController>())
	{
		m_Controller.Send(IPC::RequestID::CreateFS, id).GetAs(m_Handle);
	}

	IPC::FSHandle BaseFileSystem::GetHandle() const
	{
		return m_Handle;
	}

	bool BaseFileSystem::IsEnabled() const
	{
		return m_Controller.Send(RequestID::FSIsEnabled, m_Handle).GetAs<bool>();
	}
	void BaseFileSystem::Enable()
	{
		m_Controller.Send(RequestID::FSEnable, m_Handle);
	}
	void BaseFileSystem::Disable()
	{
		m_Controller.Send(RequestID::FSDisable, m_Handle);
	}

	wxString BaseFileSystem::GetMountPoint() const
	{
		return m_Controller.Send(RequestID::FSGetMountPoint, m_Handle).GetAs<wxString>();
	}
	void BaseFileSystem::SetMountPoint(const wxString& path)
	{
		m_Controller.Send(RequestID::FSSetMountPoint, m_Handle, path);
	}

	bool BaseFileSystem::EnableAsyncIO(bool value)
	{
		return m_Controller.Send(RequestID::FSEnableAsyncIO, m_Handle, value).GetAs<bool>();
	}
	bool BaseFileSystem::EnableExtendedSecurity(bool value)
	{
		return m_Controller.Send(RequestID::FSEnableExtendedSecurity, m_Handle, value).GetAs<bool>();
	}
	bool BaseFileSystem::EnableImpersonateCallerUser(bool value)
	{
		return m_Controller.Send(RequestID::FSEnableImpersonateCallerUser, m_Handle, value).GetAs<bool>();
	}
}
