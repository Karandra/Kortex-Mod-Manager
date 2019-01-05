#include "stdafx.h"
#include "MultiMirror.h"
#include "IPC/Common.h"

using namespace Kortex::IPC;

namespace Kortex::VirtualFileSystem
{
	MultiMirror::MultiMirror(FileSystemID id, const wxString& mountPoint, const wxString& source)
		:Convergence(id, mountPoint, source)
	{
	}
	MultiMirror::MultiMirror(const wxString& mountPoint, const wxString& source)
		:Convergence(IPC::FileSystemID::MultiMirror, mountPoint, source)
	{
	}
	MultiMirror::MultiMirror(const wxString& mountPoint, const KxStringVector& sources)
		:Convergence(IPC::FileSystemID::MultiMirror, mountPoint, sources.front())
	{
		for (size_t i = 1; i < sources.size(); i++)
		{
			AddVirtualFolder(sources[i]);
		}
	}
}
