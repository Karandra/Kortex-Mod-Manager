#pragma once
#include "stdafx.h"
#include "KVirtualFileSystemBase.h"
class KVirtualFileSystemConvergenceImpl;
class KVirtualFileSystemService;
class KxVFSBase;

class KVirtualFileSystemConvergence: public KVirtualFileSystemBase
{
	private:
		std::unique_ptr<KVirtualFileSystemConvergenceImpl> m_Impl;

	protected:
		virtual KxVFSBase* GetImpl() const override;

	public:
		KVirtualFileSystemConvergence(KVirtualFileSystemService* service, const wxString& mountPoint, const wxString& writeTarget);
		virtual ~KVirtualFileSystemConvergence();

	public:
		bool CanDeleteInVirtualFolder() const;
		bool SetCanDeleteInVirtualFolder(bool value);

		wxString GetWriteTarget() const;
		bool SetWriteTarget(const wxString& writeTarget);

		bool AddVirtualFolder(const wxString& path);
		bool ClearVirtualFolders();
};
typedef std::vector<KVirtualFileSystemConvergence*> KVirtualFileSystemConvergenceArray;
