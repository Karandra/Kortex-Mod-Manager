#include "stdafx.h"
#include "MultiMirror.h"
#include "VirtualFileSystem/IVFSService.h"
#include "VirtualFileSystem/FSControllerService.h"

namespace Kortex::VirtualFileSystem
{
	MultiMirror::MultiMirror(const wxString& mountPoint, const wxString& source)
		:KxVFSWrapper(ToKxDynamicStringRef(mountPoint), ToKxDynamicStringRef(source))
	{
	}
	MultiMirror::MultiMirror(const wxString& mountPoint, const KxStringVector& sources)
		:MultiMirror(mountPoint, sources.front())
	{
		for (size_t i = 1; i < sources.size(); i++)
		{
			AddVirtualFolder(sources[i]);
		}
	}
}
