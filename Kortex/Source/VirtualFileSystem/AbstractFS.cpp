#include "stdafx.h"
#include "AbstractFS.h"
#include "IPC/Common.h"

using namespace Kortex::IPC;

namespace Kortex::VirtualFileSystem
{
	void AbstractFS::OnEnabled()
	{
	}
	void AbstractFS::OnDisabled()
	{
	}

	AbstractFS::AbstractFS(FileSystemID id)
		:m_Controller(*IVFSService::GetInstance()->GetNativeService<FSController>())
	{
		m_Controller.Send(RequestID::CreateFS, id).GetAs(m_Handle);
	}

	Kortex::IPC::FSHandle AbstractFS::GetHandle() const
	{
		return m_Handle;
	}

	bool AbstractFS::IsEnabled() const
	{
		return m_Controller.Send(RequestID::FSIsEnabled, m_Handle).GetAs<bool>();
	}
	void AbstractFS::Enable()
	{
		m_Controller.Send(RequestID::FSEnable, m_Handle);
	}
	void AbstractFS::Disable()
	{
		m_Controller.Send(RequestID::FSDisable, m_Handle);
	}

	void AbstractFS::SetMountPoint(const wxString& path)
	{
		m_Controller.Send(RequestID::FSSetMountPoint, m_Handle, path);
	}
}
