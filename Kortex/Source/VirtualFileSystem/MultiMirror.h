#pragma once
#include "stdafx.h"
#include "Convergence.h"

namespace Kortex::VirtualFileSystem
{
	class MultiMirror: public Convergence
	{
		protected:
			MultiMirror(IPC::FileSystemID id, const wxString& mountPoint, const wxString& source);

		public:
			MultiMirror(const wxString& mountPoint, const wxString& source);
			MultiMirror(const wxString& mountPoint, const KxStringVector& sources);
	};
}
