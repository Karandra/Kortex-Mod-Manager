#pragma once
#include "stdafx.h"
#include "KxVFSBase.h"
#include "Mirror/KxVFSMirror.h"
class KVirtualFileSystemService;
class KVirtualFileSystemBase;

class KVirtualFileSystemMirrorImpl: public KxVFSMirror
{
	private:
		KVirtualFileSystemBase* m_KVFS = NULL;

	public:
		KVirtualFileSystemMirrorImpl(KVirtualFileSystemService* service, KVirtualFileSystemBase* pKVFS, const wxString& mountPoint, const wxString& source);
		virtual ~KVirtualFileSystemMirrorImpl();

	public:
		virtual NTSTATUS OnMount(DOKAN_MOUNTED_INFO* eventInfo) override;
		virtual NTSTATUS OnUnMount(DOKAN_UNMOUNTED_INFO* eventInfo) override;
};
