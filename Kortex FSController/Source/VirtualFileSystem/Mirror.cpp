#include "stdafx.h"
#include "Mirror.h"
#include "VirtualFileSystem/IVFSService.h"
#include "VirtualFileSystem/FSControllerService.h"

namespace Kortex::VirtualFileSystem
{
	Mirror::Mirror(const wxString& mountPoint, const wxString& source)
		:KxVFSWrapper(ToKxDynamicStringRef(mountPoint), ToKxDynamicStringRef(source))
	{
	}
}
