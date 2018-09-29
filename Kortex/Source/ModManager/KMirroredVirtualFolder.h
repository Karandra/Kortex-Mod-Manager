#pragma once
#include "stdafx.h"
class KVFSMirror;
class KVFSService;

class KMirroredVirtualFolder
{
	private:
		KVFSMirror* m_VFS = NULL;
		wxString m_Source;
		wxString m_Target;

	public:
		KMirroredVirtualFolder(KVFSService* service, const wxString& source, const wxString& target);
		~KMirroredVirtualFolder();

	public:
		bool IsOK() const
		{
			return m_VFS;
		}
		KVFSMirror* GetVFS() const
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
