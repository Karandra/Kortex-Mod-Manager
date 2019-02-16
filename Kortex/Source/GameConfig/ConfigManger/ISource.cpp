#include "stdafx.h"
#include "ISource.h"
#include <Kortex/ModManager.hpp>

namespace Kortex::GameConfig
{
	wxString ISource::DispatchFSLocation(const wxString& path) const
	{
		return IModDispatcher::GetInstance()->ResolveLocationPath(path);
	}
}
