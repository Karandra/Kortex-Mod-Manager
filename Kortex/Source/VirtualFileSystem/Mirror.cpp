#include "stdafx.h"
#include "Mirror.h"
#include "IPC/Common.h"

using namespace Kortex::IPC;

namespace Kortex::VirtualFileSystem
{
	Mirror::Mirror(FileSystemID id, const wxString& mountPoint, const wxString& source)
		:BaseFileSystem(id)
	{
		m_Controller.Send(RequestID::FSSetMountPoint, m_Handle, mountPoint);
		m_Controller.Send(RequestID::FSSetSource, m_Handle, source);
	}
	Mirror::Mirror(const wxString& mountPoint, const wxString& source)
		:Mirror(FileSystemID::Mirror, mountPoint, source)
	{
	}

	void Mirror::SetSource(const wxString& path)
	{
		m_Controller.Send(RequestID::FSSetSource, m_Handle, path);
	}
}
