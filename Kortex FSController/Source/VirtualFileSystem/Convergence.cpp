#include "stdafx.h"
#include "Convergence.h"
#include "VirtualFileSystem/IVFSService.h"
#include "VirtualFileSystem/FSControllerService.h"

namespace Kortex::VirtualFileSystem
{
	Convergence::Convergence(const wxString& mountPoint, const wxString& writeTarget)
		:KxVFSWrapper(ToKxDynamicStringRef(mountPoint), ToKxDynamicStringRef(writeTarget))
	{
	}
}
