#pragma once
#include "stdafx.h"
#include "Mirror.h"

namespace Kortex::VirtualFileSystem
{
	class Convergence: public Mirror
	{
		protected:
			Convergence(IPC::FileSystemID id, const wxString& mountPoint, const wxString& writeTarget);

		public:
			Convergence(const wxString& mountPoint, const wxString& writeTarget);

		public:
			void AddVirtualFolder(const wxString& path);
			size_t BuildFileTree();
	};
}
