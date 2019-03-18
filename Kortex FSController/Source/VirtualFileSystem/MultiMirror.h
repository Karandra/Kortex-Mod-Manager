#pragma once
#include "stdafx.h"
#include "Common.h"
#include "Convergence.h"
#include <KxVirtualFileSystem/ConvergenceFS.h>

namespace Kortex::VirtualFileSystem
{
	class MultiMirror: public Convergence
	{
		public:
			MultiMirror(const wxString& mountPoint = {}, const wxString& source = {});
			MultiMirror(const wxString& mountPoint, const KxStringVector& sources);
	};
}
