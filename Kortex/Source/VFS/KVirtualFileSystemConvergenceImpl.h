#pragma once
#include "stdafx.h"
#include "KxVFSBase.h"
#include "Convergence/KxVFSConvergence.h"
class KVirtualFileSystemBase;
class KVirtualFileSystemService;

class KVirtualFileSystemConvergenceImpl: public KxVFSConvergence
{
	private:
		KVirtualFileSystemBase* m_KVFS = NULL;

	public:
		KVirtualFileSystemConvergenceImpl(KVirtualFileSystemService* service, KVirtualFileSystemBase* pKVFS, const wxString& mountPoint, const wxString& writeTarget);
		virtual ~KVirtualFileSystemConvergenceImpl();

	public:
		virtual NTSTATUS OnMount(DOKAN_MOUNTED_INFO* eventInfo) override;
		virtual NTSTATUS OnUnMount(DOKAN_UNMOUNTED_INFO* eventInfo) override;
};
