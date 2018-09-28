#include "stdafx.h"
#include "KVirtualFileSystemConvergence.h"
#include "KVirtualFileSystemConvergenceImpl.h"

KxVFSBase* KVirtualFileSystemConvergence::GetImpl() const
{
	return m_Impl.get();
}

KVirtualFileSystemConvergence::KVirtualFileSystemConvergence(KVirtualFileSystemService* service, const wxString& mountPoint, const wxString& writeTarget)
	:KVirtualFileSystemBase(service)
{
	m_Impl = std::make_unique<KVirtualFileSystemConvergenceImpl>(service, this, mountPoint, writeTarget);
}
KVirtualFileSystemConvergence::~KVirtualFileSystemConvergence()
{
}

bool KVirtualFileSystemConvergence::CanDeleteInVirtualFolder() const
{
	return m_Impl->CanDeleteInVirtualFolder();
}
bool KVirtualFileSystemConvergence::SetCanDeleteInVirtualFolder(bool value)
{
	return m_Impl->SetCanDeleteInVirtualFolder(value);
}

wxString KVirtualFileSystemConvergence::GetWriteTarget() const
{
	return m_Impl->GetWriteTarget();
}
bool KVirtualFileSystemConvergence::SetWriteTarget(const wxString& writeTarget)
{
	return m_Impl->SetWriteTarget(writeTarget);
}

bool KVirtualFileSystemConvergence::AddVirtualFolder(const wxString& path)
{
	return m_Impl->AddVirtualFolder(path);
}
bool KVirtualFileSystemConvergence::ClearVirtualFolders()
{
	return m_Impl->ClearVirtualFolders();
}

void KVirtualFileSystemConvergence::BuildDispatcherIndex()
{
	m_Impl->BuildDispatcherIndex();
}
void KVirtualFileSystemConvergence::SetDispatcherIndex(const ExternalDispatcherIndexT& index)
{
	m_Impl->SetDispatcherIndex(index);
}
