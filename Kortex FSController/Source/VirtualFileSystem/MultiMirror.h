#pragma once
#include "stdafx.h"
#include "Common.h"
#include "KxVFSWrapper.h"
#include <KxVirtualFileSystem/AbstractFS.h>
#include <KxVirtualFileSystem/Convergence/ConvergenceFS.h>
#include <KxVirtualFileSystem/MultiMirror/MultiMirrorFS.h>

namespace Kortex::VirtualFileSystem
{
	class MultiMirror: public KxVFSWrapper<KxVFS::MultiMirrorFS>
	{
		public:
			MultiMirror(const wxString& mountPoint = {}, const wxString& source = {});
			MultiMirror(const wxString& mountPoint, const KxStringVector& sources);

		public:
			void AddVirtualFolder(const wxString& path)
			{
				KxVFS::MultiMirrorFS::AddVirtualFolder(ToKxDynamicStringRef(path));
			}
	};
}
