#include "stdafx.h"
#include "KIPCRequest.h"

//////////////////////////////////////////////////////////////////////////
KIPCRequest_CreateMirrorVFS::KIPCRequest_CreateMirrorVFS(const wxString& source, const wxString& target)
	:m_Source(source), m_Target(target)
{
}

//////////////////////////////////////////////////////////////////////////
KIPCRequest_CreateConvergenceVFS::KIPCRequest_CreateConvergenceVFS(const wxString& mountPoint, const wxString& writeTarget, bool canDeleteInVirtualFolder)
	: m_MountPoint(mountPoint), m_WriteTarget(writeTarget), m_CanDeleteInVirtualFolder(canDeleteInVirtualFolder)
{
}
