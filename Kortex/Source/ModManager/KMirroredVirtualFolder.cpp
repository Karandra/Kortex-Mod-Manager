#include "stdafx.h"
#include "KMirroredVirtualFolder.h"
#include "VFS/KVFSMirror.h"

KMirroredVirtualFolder::KMirroredVirtualFolder(KVFSService* service, const wxString& source, const wxString& target)
	:m_Source(source), m_Target(target)
{
	m_VFS = new KVFSMirror(service, target, source);
}
KMirroredVirtualFolder::~KMirroredVirtualFolder()
{
	delete m_VFS;
}
