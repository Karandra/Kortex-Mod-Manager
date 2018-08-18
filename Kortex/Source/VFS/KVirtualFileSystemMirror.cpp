#include "stdafx.h"
#include "KVirtualFileSystemMirror.h"
#include "KVirtualFileSystemMirrorImpl.h"

KxVFSBase* KVirtualFileSystemMirror::GetImpl() const
{
	return m_Impl.get();
}

KVirtualFileSystemMirror::KVirtualFileSystemMirror(KVirtualFileSystemService* service, const wxString& mountPoint, const wxString& source)
	:KVirtualFileSystemBase(service)
{
	m_Impl = std::make_unique<KVirtualFileSystemMirrorImpl>(service, this, mountPoint, source);
}
KVirtualFileSystemMirror::~KVirtualFileSystemMirror()
{
}

wxString KVirtualFileSystemMirror::GetSource() const
{
	return m_Impl->GetSource();
}
bool KVirtualFileSystemMirror::SetSource(const wxString& source)
{
	return m_Impl->SetSource(source);
}
