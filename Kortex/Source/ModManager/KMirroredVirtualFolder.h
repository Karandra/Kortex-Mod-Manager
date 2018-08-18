#pragma once
#include "stdafx.h"
class KVirtualFileSystemMirror;
class KVirtualFileSystemService;

class KMirroredVirtualFolder
{
	private:
		KVirtualFileSystemMirror* m_VFS = NULL;
		wxString m_Source;
		wxString m_Target;

	public:
		KMirroredVirtualFolder(KVirtualFileSystemService* service, const wxString& source, const wxString& target);
		~KMirroredVirtualFolder();

	public:
		bool IsOK() const
		{
			return m_VFS;
		}
		KVirtualFileSystemMirror* GetVFS() const
		{
			return m_VFS;
		}

		const wxString& GetSource() const
		{
			return m_Source;
		}
		const wxString& GetDestination() const
		{
			return m_Target;
		}
};
typedef std::vector<KMirroredVirtualFolder*> KMirroredVirtualFolderArray;
