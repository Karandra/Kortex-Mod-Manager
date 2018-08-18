#pragma once
#include "stdafx.h"
#include "KVirtualFileSystemBase.h"
class KVirtualFileSystemMirrorImpl;
class KVirtualFileSystemService;
class KxVFSBase;

class KVirtualFileSystemMirror: public KVirtualFileSystemBase
{
	private:
		std::unique_ptr<KVirtualFileSystemMirrorImpl> m_Impl;

	protected:
		virtual KxVFSBase* GetImpl() const override;

	public:
		KVirtualFileSystemMirror(KVirtualFileSystemService* service, const wxString& mountPoint, const wxString& source);
		virtual ~KVirtualFileSystemMirror();

	public:
		wxString GetSource() const;
		bool SetSource(const wxString& source);
};
typedef std::vector<KVirtualFileSystemMirror*> KVirtualFileSystemMirrorArray;
