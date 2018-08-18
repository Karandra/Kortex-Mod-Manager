#include "stdafx.h"
#include "KMirroredVirtualFolder.h"
#include "VFS/KVirtualFileSystemMirror.h"

KMirroredVirtualFolder::KMirroredVirtualFolder(KVirtualFileSystemService* service, const wxString& source, const wxString& target)
	:m_Source(source), m_Target(target)
{
	m_VFS = new KVirtualFileSystemMirror(service, target, source);
}
KMirroredVirtualFolder::~KMirroredVirtualFolder()
{
	delete m_VFS;
}
